#ifndef DISTANCEFUNCTIONS_H
#define DISTANCEFUNCTIONS_H

#include <cmath>

template <typename VecType>
double euclidianDistance(const VecType &a, const VecType &b)
{
	double v = 0.0;

	for(typename VecType::const_iterator ita = a.begin(), itb = b.begin(); ita != a.end(); ++ita, ++itb)
		v += ((*ita) - (*itb)) * ((*ita) - (*itb));

	return sqrt(v);
}

double reciprocalSimilarity(const double d)
{
	return 1.0 / (d + 1.0);
}

double exponentialSimilarity(const double d, const double normalizationFactor)
{
	return exp(- d * d / normalizationFactor);
}

#endif /* DISTANCEFUNCTIONS_H */
