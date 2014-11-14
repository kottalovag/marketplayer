#include "strategymapper.h"

const QString oppositeParetoValue = "opposite pareto";
const QString randomParetoValue = "random pareto";
const QString randomTriangleValue = "random triangle";

const QString alwaysValue = "always";
const QString higherGainValue = "want higher gain";
const QString higherProportionValue = "want higher proportion";

QString OfferStrategyNameVisitor::getStrategyDescription(AbstractOfferStrategy &s) {
    s.accept(*this);
    return visitedStrategy;
}

void OfferStrategyNameVisitor::visit(OppositeParetoOfferStrategy &) {
    visitedStrategy = "opposite pareto";
}

void OfferStrategyNameVisitor::visit(RandomParetoOfferStrategy &) {
    visitedStrategy = "random pareto";
}

void OfferStrategyNameVisitor::visit(RandomTriangleOfferStrategy &) {
    visitedStrategy = "random triangle";
}


QString AcceptanceStrategyNameVisitor::getStrategyDescription(AbstractAcceptanceStrategy &s) {
    s.accept(*this);
    return visitedStrategy;
}

void AcceptanceStrategyNameVisitor::visit(AlwaysAcceptanceStrategy &) {
    visitedStrategy = "always";
}

void AcceptanceStrategyNameVisitor::visit(HigherGainAcceptanceStrategy &) {
    visitedStrategy = "want higher gain";
}

void AcceptanceStrategyNameVisitor::visit(HigherProportionAcceptanceStrategy &) {
    visitedStrategy = "want higher proportion";
}

unique_ptr<AbstractOfferStrategy> createOfferStrategy(QString name)
{
    unique_ptr<AbstractOfferStrategy> result;
    if (name == oppositeParetoValue) {
        result.reset(new OppositeParetoOfferStrategy());
    } else if (name == randomParetoValue) {
        result.reset(new RandomParetoOfferStrategy());
    } else if (name == randomTriangleValue) {
        result.reset(new RandomTriangleOfferStrategy());
    } else {
        Q_ASSERT(false);
    }
    return result;
}

unique_ptr<AbstractAcceptanceStrategy> createAcceptanceStrategy(QString name)
{
    unique_ptr<AbstractAcceptanceStrategy> result;
    if (name == alwaysValue) {
        result.reset(new AlwaysAcceptanceStrategy());
    } else if (name == higherGainValue) {
        result.reset(new HigherGainAcceptanceStrategy());
    } else if (name == higherProportionValue) {
        result.reset(new HigherProportionAcceptanceStrategy());
    } else {
        Q_ASSERT(false);
    }
    return result;
}
