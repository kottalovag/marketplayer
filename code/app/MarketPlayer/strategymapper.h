#ifndef STRATEGYMAPPER_H
#define STRATEGYMAPPER_H


#include "strategy.h"
#include <QString>
#include <memory>

using std::unique_ptr;

extern const QString oppositeParetoValue;
extern const QString randomParetoValue;
extern const QString randomTriangleValue;

extern const QString alwaysValue;
extern const QString higherGainValue;
extern const QString higherProportionValue;

struct OfferStrategyNameVisitor : IOfferStrategyVisitor
{
    QString getStrategyDescription(AbstractOfferStrategy& s);
    virtual void visit (OppositeParetoOfferStrategy&);
    virtual void visit (RandomParetoOfferStrategy&);
    virtual void visit (RandomTriangleOfferStrategy&);
private:
    QString visitedStrategy;
};

struct AcceptanceStrategyNameVisitor : IAcceptanceStrategyVisitor
{
    QString getStrategyDescription(AbstractAcceptanceStrategy& s);
    virtual void visit (AlwaysAcceptanceStrategy&);
    virtual void visit (HigherGainAcceptanceStrategy&);
    virtual void visit (HigherProportionAcceptanceStrategy&);
private:
    QString visitedStrategy;
};

unique_ptr<AbstractOfferStrategy> createOfferStrategy(QString name);
unique_ptr<AbstractAcceptanceStrategy> createAcceptanceStrategy(QString name);

#endif // STRATEGYMAPPER_H
