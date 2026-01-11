#include <QQmlApplicationEngine>
#include "StickerManager.hpp"

extern "C" void registerQmldiff() {
	qmlRegisterSingletonInstance<StickerManager>(
			"dk.ingatellent.stickers", 1, 0, "StickerManager", new StickerManager());
}
