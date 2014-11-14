#include "strategy.h"
#include "model.h"

Position OppositeParetoOfferStrategy::propose(EdgeworthSituation const& situation, URNG&) const
{
    Position const p2 = situation.calculateCurve2ParetoIntersection();
    //debugShowPoint(p2);
    return p2;
}

Position RandomParetoOfferStrategy::propose(EdgeworthSituation const& situation, URNG& rng) const
{
    Position const p2 = situation.calculateCurve2ParetoIntersection();
    Position const p1 = situation.calculateCurve1ParetoIntersection();
    double const factor = std::uniform_real_distribution<double>(0.0, 1.0)(rng);
    auto const result = p1 + (p2 - p1) * factor;
    //debugShowPoint(result);
    return result;
}

Position RandomTriangleOfferStrategy::propose(EdgeworthSituation const& situation, URNG& rng) const
{
    Position const p0 = situation.getFixPoint();
    Position const p1 = situation.calculateCurve1ParetoIntersection();
    Position const p2 = situation.calculateCurve2ParetoIntersection();

    std::uniform_real_distribution<double> udist(0.0, 1.0);

    //we pick a point as if in a paralelogram for sake of uniformity
    auto const v01 = (p1 - p0);
    auto const v02 = (p2 - p0);
    double const factor1 = udist(rng);
    double const factor2 = udist(rng);
    Position px = p0 + v01*factor1 + v02*factor2;

    //if the point falls in the wrong half
    //  then we do point reflection around p1-p2 side's midde point
    //  to end up in the desired half
    if (!isPointInTriangle(p0, p1, p2, px)) {
        auto origo = p1 + (p2 - p1) * 0.5;
        px = px + (origo - px) * 2.0;
    }
    //debugShowPoint(px);
    return px;
}

bool AlwaysAcceptanceStrategy::consider(EdgeworthSituation const&) const
{
    return true;
}

bool AbstractAcceptanceStrategy::considerGeneral(EdgeworthSituation const& situation,
                                          std::function<Amount_t(const Amount_t &, const Amount_t &)> evaluate) const
{
    auto const actor1Utility = situation.calculateOriginalUtility(situation.actor1);
    auto const actor2Utility = situation.calculateOriginalUtility(situation.actor2);
    auto const actor1NewUtility = situation.calculateNewUtilityActor1();
    auto const actor2NewUtility = situation.calculateNewUtilityActor2();
    auto const actor1Gain = evaluate(actor1NewUtility, actor1Utility);
    auto const actor2Gain = evaluate(actor2NewUtility, actor2Utility);
    return actor1Gain <= actor2Gain;
}

bool HigherGainAcceptanceStrategy::consider(EdgeworthSituation const& situation) const
{
    return considerGeneral(situation,
                           [](Amount_t const& newUtility, Amount_t const& originalUtility) {
        return newUtility - originalUtility;
    });
}

bool HigherProportionAcceptanceStrategy::consider(EdgeworthSituation const& situation) const
{
    return considerGeneral(situation,
                           [](Amount_t const& newUtility, Amount_t const& originalUtility) {
        return newUtility / originalUtility;
    });
}
