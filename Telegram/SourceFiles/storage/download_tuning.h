/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#pragma once

#include <QtCore/QCoreApplication>
#include <QtCore/QFile>
#include <QtCore/QIODevice>
#include <QtCore/QString>

#include <array>

namespace Storage {
namespace DownloadTuning {

// Tuning level is read once from "ayu_dl_tune.txt" placed next to the
// executable. The file must contain a single number:
//
//   0 = official behaviour (no boost at all)
//   1 = balanced
//   2 = aggressive  (default; recommended behind a proxy / high RTT link)
//   3 = maximum
//
// If the file is missing or unreadable, level 2 is used. Changing the file
// requires a restart: session counts are fixed when a DC balance is created.
inline int Level() {
	static const auto level = [] {
		auto file = QFile(
			QCoreApplication::applicationDirPath()
			+ QStringLiteral("/ayu_dl_tune.txt"));
		if (!file.open(QIODevice::ReadOnly)) {
			return 2;
		}
		auto ok = false;
		const auto value = QString::fromUtf8(file.readAll())
			.trimmed()
			.toInt(&ok);
		return (ok && value >= 0 && value <= 3) ? value : 2;
	}();
	return level;
}

struct Profile {
	int startSessions;
	int maxSessions;
	int startWaitedParts;
	int maxWaitedParts;
	int addSessionTimeout;
	int addSessionSuccesses;
	int removeAfterTimeouts;
	int badRequestSeconds;
	int preloadParts;
};

inline const Profile &Current() {
	static constexpr auto kProfiles = std::array<Profile, 4>{{
		// Official. Kept so the change can be turned off completely.
		{ 1, 8, 4, 16, 8, 3, 4, 8, 32 },
		// Balanced: more sessions, wider window, tolerant of slow requests.
		{ 3, 12, 8, 24, 4, 2, 8, 15, 48 },
		// Aggressive: default. Proxy links have a large delay-bandwidth
		// product, so what matters is a big starting window and not
		// mistaking a slow link for a broken one.
		{ 6, 16, 12, 32, 2, 1, 10, 20, 64 },
		// Maximum. Only useful on a fast, low-loss proxy node.
		{ 8, 24, 16, 32, 2, 1, 12, 25, 96 },
	}};
	return kProfiles[Level()];
}

} // namespace DownloadTuning
} // namespace Storage
