#include "casenamemanager.h"

CaseNameManager::CaseNameManager()
{
    reset();
}

QString CaseNameManager::provideNextCaseName()
{
    return QString("Case ") + QString::number(idx++);
}

void CaseNameManager::reset()
{
    idx = 1;
}
