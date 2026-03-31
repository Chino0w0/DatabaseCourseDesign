#include "MyHealthService.h"

#include "../services/DiseaseService.h"
#include "../services/HealthService.h"
#include "../services/VisitService.h"


json MyHealthService::getProfile(int residentId) {
  if (residentId <= 0) {
    return json{{"error", "当前居民账号未绑定居民档案"}, {"not_bound", true}};
  }

  HealthService healthService;
  return healthService.getHealthRecord(residentId);
}

json MyHealthService::getMeasurements(int residentId, int limit) {
  if (residentId <= 0) {
    return json{{"error", "当前居民账号未绑定居民档案"}, {"not_bound", true}};
  }

  HealthService healthService;
  return healthService.getMeasurements(residentId, limit);
}

json MyHealthService::getDiseases(int residentId) {
  if (residentId <= 0) {
    return json{{"error", "当前居民账号未绑定居民档案"}, {"not_bound", true}};
  }

  DiseaseService diseaseService;
  return diseaseService.getResidentDiseases(residentId);
}

json MyHealthService::getVisits(int residentId) {
  if (residentId <= 0) {
    return json{{"error", "当前居民账号未绑定居民档案"}, {"not_bound", true}};
  }

  VisitService visitService;
  return visitService.getVisits(residentId);
}
