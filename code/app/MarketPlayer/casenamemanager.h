#ifndef CASENAMEMANAGER_H
#define CASENAMEMANAGER_H

#include <QString>

struct CaseNameManager
{
    CaseNameManager();
    QString provideNextCaseName();
    void reset();

private:
    int idx;
};

#endif // CASENAMEMANAGER_H
