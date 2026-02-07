#include <QQmlApplicationEngine>
#include "StickerManager.hpp"

extern "C" void registerQmldiff() {
	qmlRegisterSingletonInstance<StickerManager>(
			"dk.ingatellent.stickers", 0, 1, "StickerManager", new StickerManager());
}
