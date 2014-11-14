#ifndef STRATEGY_H
#define STRATEGY_H

#include "modelutils.h"

struct EdgeworthSituation;

struct OppositeParetoOfferStrategy;
struct RandomParetoOfferStrategy;
struct RandomTriangleOfferStrategy;

struct IOfferStrategyVisitor
{
    virtual void visit(OppositeParetoOfferStrategy& s) = 0;
    virtual void visit(RandomParetoOfferStrategy& s) = 0;
    virtual void visit(RandomTriangleOfferStrategy& s) = 0;
};

struct AbstractOfferStrategy
{
    virtual AbstractOfferStrategy* clone() const = 0;
    virtual void accept(IOfferStrategyVisitor& v) = 0;
    virtual Position propose(EdgeworthSituation const& situation, URNG& rng) const = 0;
    virtual ~AbstractOfferStrategy(){}
};

struct OppositeParetoOfferStrategy: AbstractOfferStrategy
{
    CLONEABLE(OppositeParetoOfferStrategy)
    VISITABLE_BY(IOfferStrategyVisitor)
    virtual Position propose(EdgeworthSituation const& situation, URNG& rng) const override;
    virtual ~OppositeParetoOfferStrategy() {}
};

struct RandomParetoOfferStrategy: AbstractOfferStrategy
{
    CLONEABLE(RandomParetoOfferStrategy)
    VISITABLE_BY(IOfferStrategyVisitor)
    virtual Position propose(EdgeworthSituation const& situation, URNG &rng) const override;
    virtual ~RandomParetoOfferStrategy() {}
};

struct RandomTriangleOfferStrategy: AbstractOfferStrategy
{
    CLONEABLE(RandomTriangleOfferStrategy)
    VISITABLE_BY(IOfferStrategyVisitor)
    virtual Position propose(EdgeworthSituation const& situation, URNG& rng) const override;
    virtual ~RandomTriangleOfferStrategy() {}
};

struct AlwaysAcceptanceStrategy;
struct HigherGainAcceptanceStrategy;
struct HigherProportionAcceptanceStrategy;

struct IAcceptanceStrategyVisitor
{
    virtual void visit(AlwaysAcceptanceStrategy& s) = 0;
    virtual void visit(HigherGainAcceptanceStrategy& s) = 0;
    virtual void visit(HigherProportionAcceptanceStrategy& s) = 0;
};

struct AbstractAcceptanceStrategy
{
    virtual AbstractAcceptanceStrategy* clone() const = 0;
    virtual void accept(IAcceptanceStrategyVisitor& v) = 0;
    virtual bool consider(EdgeworthSituation const& situation) const = 0;
    virtual ~AbstractAcceptanceStrategy(){}
    bool considerGeneral(const EdgeworthSituation &situation,
                         std::function<Amount_t(Amount_t const&, Amount_t const&)> evaluate) const;
};

struct AlwaysAcceptanceStrategy: AbstractAcceptanceStrategy
{
    CLONEABLE(AlwaysAcceptanceStrategy)
    VISITABLE_BY(IAcceptanceStrategyVisitor)
    virtual bool consider(EdgeworthSituation const& situation) const override;
    virtual ~AlwaysAcceptanceStrategy(){}
};

struct HigherGainAcceptanceStrategy: AbstractAcceptanceStrategy
{
    CLONEABLE(HigherGainAcceptanceStrategy)
    VISITABLE_BY(IAcceptanceStrategyVisitor)
    virtual bool consider(EdgeworthSituation const& situation) const override;
    virtual ~HigherGainAcceptanceStrategy(){}
};

struct HigherProportionAcceptanceStrategy: AbstractAcceptanceStrategy
{
    CLONEABLE(HigherProportionAcceptanceStrategy)
    VISITABLE_BY(IAcceptanceStrategyVisitor)
    virtual bool consider(EdgeworthSituation const& situation) const override;
    virtual ~HigherProportionAcceptanceStrategy(){}
};

#endif // STRATEGY_H
