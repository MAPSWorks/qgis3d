#ifndef MAPTEXTUREGENERATOR_H
#define MAPTEXTUREGENERATOR_H

class QgsMapRendererSequentialJob;
class QgsMapSettings;
class QgsProject;
class QgsRasterLayer;

#include <QObject>

#include "tilingscheme.h"

#include "qgsrectangle.h"

struct Map3D;

/**
 * Responsible for:
 * - rendering map tiles in background
 * - caching tiles on disk (?)
 */
class MapTextureGenerator : public QObject
{
  Q_OBJECT
public:
  MapTextureGenerator(const Map3D& map);

  //! Start async rendering of a map for the given extent (must be a square!)
  //! Returns job ID
  int render(const QgsRectangle& extent, const QString& debugText = QString());

  //! Cancels a rendering job
  void cancelJob(int jobId);

signals:
  void tileReady(int jobId, const QImage& image);

private slots:
  void onRenderingFinished();

private:
  QgsMapSettings baseMapSettings();

  const Map3D& map;

  struct JobData
  {
    int jobId;
    QgsMapRendererSequentialJob* job;
    QgsRectangle extent;
    QString debugText;
  };

  QHash<QgsMapRendererSequentialJob*, JobData> jobs;
  int lastJobId;
};


#endif // MAPTEXTUREGENERATOR_H
