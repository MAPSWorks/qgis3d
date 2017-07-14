#include "maptexturegenerator.h"

#include <qgsmaprenderersequentialjob.h>
#include <qgsmapsettings.h>
#include <qgsproject.h>

#include "map3d.h"

MapTextureGenerator::MapTextureGenerator(const Map3D& map)
  : map(map)
  , lastJobId(0)
{
}

int MapTextureGenerator::render(const QgsRectangle &extent, const QString &debugText)
{
  QgsMapSettings mapSettings(baseMapSettings());
  mapSettings.setExtent(extent);

  QgsMapRendererSequentialJob* job = new QgsMapRendererSequentialJob(mapSettings);
  connect(job, &QgsMapRendererJob::finished, this, &MapTextureGenerator::onRenderingFinished);
  job->start();

  JobData jobData;
  jobData.jobId = ++lastJobId;
  jobData.job = job;
  jobData.extent = extent;
  jobData.debugText = debugText;

  jobs.insert(job, jobData);
  //qDebug() << "added job: " << jobData.jobId << "  .... in queue: " << jobs.count();
  return jobData.jobId;
}

void MapTextureGenerator::cancelJob(int jobId)
{
  Q_FOREACH(const JobData& jd, jobs)
  {
    if (jd.jobId == jobId)
    {
      //qDebug() << "cancelling job " << jobId;
      jd.job->cancelWithoutBlocking();
      disconnect(jd.job, &QgsMapRendererJob::finished, this, &MapTextureGenerator::onRenderingFinished);
      jd.job->deleteLater();
      jobs.remove(jd.job);
      return;
    }
  }
  Q_ASSERT(false && "requested job ID does not exist!");
}


void MapTextureGenerator::onRenderingFinished()
{
  QgsMapRendererSequentialJob* mapJob = static_cast<QgsMapRendererSequentialJob*>(sender());

  Q_ASSERT(jobs.contains(mapJob));
  JobData jobData = jobs.value(mapJob);

  QImage img = mapJob->renderedImage();

  if (!jobData.debugText.isEmpty())
  {
    // extra tile information for debugging
    QPainter p(&img);
    p.setPen(Qt::white);
    p.drawRect(0,0,img.width()-1, img.height()-1);
    p.drawText(img.rect(), jobData.debugText, QTextOption(Qt::AlignCenter));
    p.end();
  }

  mapJob->deleteLater();
  jobs.remove(mapJob);

  //qDebug() << "finished job " << jobData.jobId << "  ... in queue: " << jobs.count();

  // pass QImage further
  emit tileReady(jobData.jobId, img);
}

QgsMapSettings MapTextureGenerator::baseMapSettings()
{
  QgsMapSettings mapSettings;
  mapSettings.setLayers(map.layers());
  mapSettings.setOutputSize(QSize(map.tileTextureSize,map.tileTextureSize));
  mapSettings.setDestinationCrs(map.crs);
  mapSettings.setBackgroundColor(Qt::gray);
  return mapSettings;
}
