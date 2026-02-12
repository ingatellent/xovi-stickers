#include "StickerManager.hpp"

#include <QRandomGenerator>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDir>
#include <QFile>
#include <QUrl>
#include <QPolygonF>
#include <QTextStream>
#include <QVariantMap>
#include <cmath>

#include "rm_SceneLineItem.hpp"


struct Coordinate {
	float x;
	float y;
};

void StickerManager::sleepMs(int ms) { ::usleep(ms*1000); }

bool StickerManager::setupVtablePtr(const QList<std::shared_ptr<SceneItem>>& items){
	if(items.empty()) return false;
	auto* item = reinterpret_cast<SceneLineItem*>(items.first().get());
	SceneLineItem::setupVtable(item->vtable);
	return true;
}

Line StickerManager::createLine(const QPointF& start, const QPointF& end) {
	QList<LinePoint> linePoints {
		{static_cast<float>(start.x()), static_cast<float>(start.y()), 25, 25, 0, 255},
			{static_cast<float>(end.x()),   static_cast<float>(end.y()),   25, 25, 0, 255}
	};
	QRectF bounds = { std::min(start.x(), end.x()), std::min(start.y(), end.y()),
		std::abs(end.x() - start.x()), std::abs(end.y() - start.y()) };
	return Line::fromPoints(std::move(linePoints), bounds);
}

inline void generateCircle(float radius, Coordinate center, LinePoint* destination, size_t count)
{
	for (size_t i = 0; i < count; i++) {
		float angle = (static_cast<float>(i) / float(count - 1)) * 2.0f * 3.14159265f;
		destination[i] = {
			static_cast<float>(center.x + radius * cos(angle)),
			static_cast<float>(center.y + radius * sin(angle)),
			25, 25, 0, 255
		};
	}   
}   

Line StickerManager::createCircle(const QPointF& _center, float radius) {
	QList<LinePoint> circlePoints(100);
	auto center = Coordinate{static_cast<float>(_center.x()), static_cast<float>(_center.y())};
	generateCircle(radius, center, circlePoints.data(), circlePoints.size());
	return Line::fromPoints(std::move(circlePoints), _center, radius * 1.1);
}




void StickerManager::saveSceneItems(const QList<std::shared_ptr<SceneItem>>& items, const QString& filename) {
	QJsonArray jsonArray;

	for (const auto& itemPtr : items) {
		auto* lineItem = reinterpret_cast<SceneLineItem*>(itemPtr.get());
		if (!lineItem) continue;

		const Line& line = lineItem->line;
		QJsonArray pointsArray;
		for (const auto& pt : line.points) {
			QJsonArray ptArr = { pt.x, pt.y, pt.speed, pt.width, pt.direction, pt.pressure };
			pointsArray.append(ptArr);
		}

		QJsonObject obj;
		obj["points"] = pointsArray;
		obj["rgba"] = static_cast<qint64>(line.rgba);   // store color as hex string
		obj["color"] = line.color;
		obj["bounds"] = QJsonArray{ line.bounds.x(), line.bounds.y(),
			line.bounds.width(), line.bounds.height() };
		obj["tool"] = line.tool;
		obj["maskScale"] = line.maskScale;
		obj["thickness"] = line.thickness;
		jsonArray.append(obj);
	}

	QJsonDocument doc(jsonArray);
	QFile file(filename);
	if (file.open(QIODevice::WriteOnly)) {
		file.write(doc.toJson());
		file.close();
	}
}

QList<std::shared_ptr<SceneItem>> StickerManager::loadSceneItems(const QString& filename) {
	QList<std::shared_ptr<SceneItem>> items;
	QFile file(filename);
	if (!file.open(QIODevice::ReadOnly)) return items;

	QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
	QJsonArray jsonArray = doc.array();

	for (const auto& val : jsonArray) {
		QJsonObject obj = val.toObject();
		QJsonArray pointsArray = obj["points"].toArray();
		QList<LinePoint> linePoints;

		for (const auto& ptVal : pointsArray) {
			QJsonArray ptArr = ptVal.toArray();
			LinePoint pt;
			pt.x = ptArr[0].toDouble();
			pt.y = ptArr[1].toDouble();
			pt.speed = static_cast<unsigned short>(ptArr[2].toInt());
			pt.width = static_cast<unsigned short>(ptArr[3].toInt());
			pt.direction = static_cast<unsigned char>(ptArr[4].toInt());
			pt.pressure = static_cast<unsigned char>(ptArr[5].toInt());
			linePoints.append(pt);
		}

		QJsonArray boundsArr = obj["bounds"].toArray();
		QRectF bounds(boundsArr[0].toDouble(), boundsArr[1].toDouble(),
				boundsArr[2].toDouble(), boundsArr[3].toDouble());

		Line line;
		line.points = std::move(linePoints);
		line.bounds = bounds;

		line.rgba = static_cast<quint32>(obj["rgba"].toInteger(0xFF000000));
		line.color = obj["color"].toInt(0x00);

		line.tool = obj["tool"].toInt(0x17);
		line.maskScale = obj["maskScale"].toDouble(1.0);
		line.thickness = obj["thickness"].toDouble(0.0);
		items.push_back(std::make_shared<SceneLineItem>(SceneLineItem::fromLine(std::move(line))));
	}

	return items;
}

Q_INVOKABLE QList<std::shared_ptr<SceneItem>> StickerManager::setColorOnSceneItems(
		const QList<std::shared_ptr<SceneItem>>& items, uint colorEnum, quint32 rgba) 
{
	QList<std::shared_ptr<SceneItem>> reColored;
	for (auto& itemPtr : items) {
		auto* lineItem = reinterpret_cast<SceneLineItem*>(itemPtr.get());
		if (!lineItem) continue;

		auto newLineItem = std::make_shared<SceneLineItem>(*lineItem);
		newLineItem->line.color = colorEnum;
		newLineItem->line.rgba = rgba;
		reColored.push_back(newLineItem);
	}
	return reColored;
}

Q_INVOKABLE QList<std::shared_ptr<SceneItem>> StickerManager::setToolOnSceneItems(
		const QList<std::shared_ptr<SceneItem>>& items, uint toolEnum)
{
	QList<std::shared_ptr<SceneItem>> retooled;
	for (auto& itemPtr : items) {
		auto* lineItem = reinterpret_cast<SceneLineItem*>(itemPtr.get());
		if (!lineItem) continue;

		auto newLineItem = std::make_shared<SceneLineItem>(*lineItem);
		newLineItem->line.tool = toolEnum;
		retooled.push_back(newLineItem);
	}
	return retooled;
}

Q_INVOKABLE QList<std::shared_ptr<SceneItem>> StickerManager::increaseThicknessOnSceneItems(
		const QList<std::shared_ptr<SceneItem>>& items)
{
	QList<std::shared_ptr<SceneItem>> resized;
	for (auto& itemPtr : items) {
		auto* lineItem = reinterpret_cast<SceneLineItem*>(itemPtr.get());
		if (!lineItem) continue;

		auto newLineItem = std::make_shared<SceneLineItem>(*lineItem);
		for (auto& pt : newLineItem->line.points) {
			pt.width = static_cast<unsigned short> (std::ceil(pt.width * 1.25F)); 
		}	
		resized.push_back(newLineItem);
	}
	return resized;
}

Q_INVOKABLE QList<std::shared_ptr<SceneItem>> StickerManager::decreaseThicknessOnSceneItems(
		const QList<std::shared_ptr<SceneItem>>& items)
{
	QList<std::shared_ptr<SceneItem>> resized;
	for (auto& itemPtr : items) {
		auto* lineItem = reinterpret_cast<SceneLineItem*>(itemPtr.get());
		if (!lineItem) continue;

		auto newLineItem = std::make_shared<SceneLineItem>(*lineItem);
		for (auto& pt : newLineItem->line.points) {
			pt.width = std::max<unsigned short>(1, pt.width*0.8f);
		}
		resized.push_back(newLineItem);
	}
	return resized;
}

Q_INVOKABLE QList<std::shared_ptr<SceneItem>> StickerManager::setThicknessOnSceneItems(
		const QList<std::shared_ptr<SceneItem>>& items, const quint32 thickness)
{
	QList<std::shared_ptr<SceneItem>> resized;
	for (auto& itemPtr : items) {
		auto* lineItem = reinterpret_cast<SceneLineItem*>(itemPtr.get());
		if (!lineItem) continue;

		auto newLineItem = std::make_shared<SceneLineItem>(*lineItem);
		for (auto& pt : newLineItem->line.points) {
			pt.width = thickness;
		}
		resized.push_back(newLineItem);
	}
	return resized;
}

void StickerManager::ensureDirectory(const QString& path) {
	QDir dir(path);
	if (!dir.exists())
		dir.mkpath(".");
}

bool StickerManager::deleteFile(const QString& path) {
	return QFile::remove(path);
}


Q_INVOKABLE QVariantMap StickerManager::getPenInfoOfFirstItem(
        const QList<std::shared_ptr<SceneItem>>& items)
{
    QVariantMap info;
    if (items.isEmpty()) return info;

    auto* lineItem = reinterpret_cast<SceneLineItem*>(items.first().get());
    if (!lineItem) return info;

    info["currentTool"] = static_cast<int>(lineItem->line.tool);
    info["currentThickness"] = lineItem->line.points.isEmpty()
                               ? 0
                               : static_cast<int>(lineItem->line.points.first().width);
    info["currentColorCode"] = static_cast<int>(lineItem->line.color);
    info["currentRgb"] = static_cast<quint32>(lineItem->line.rgba);

    return info;
}

