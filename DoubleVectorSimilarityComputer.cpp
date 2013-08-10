#include <tulip/ImportModule.h>
#include <tulip/TulipPluginHeaders.h>
#include <tulip/StringCollection.h>
#include <math.h>
#include <stdexcept>

#include "SimilarityFunctions.h"

using namespace std;
using namespace tlp;

#define CHECK_PROP_PROVIDED(PROP, STOR) \
	do { \
		if(!dataSet->get(PROP, STOR)) \
			throw std::runtime_error(std::string("No \"") + PROP + "\" property provided."); \
	} while(0)

const string PLUGIN_NAME("Compute connected nodes similarity on DoubleVector");

namespace {
const char *paramHelp[] = {
	// 0 Feature property
	HTML_HELP_OPEN()
		HTML_HELP_DEF("Type", "DoubleVectorProperty")
		HTML_HELP_DEF("Default", "data")
		HTML_HELP_BODY()
		"The property holding the data to use to compute the similarity."
		HTML_HELP_CLOSE(),

	// 1 Distance function
	HTML_HELP_OPEN()
		HTML_HELP_DEF("Type", "StringCollection")
		HTML_HELP_DEF("Values", "Euclidian")
		HTML_HELP_DEF("Default", "Euclidian")
		HTML_HELP_BODY()
		"The distance measure to use."
		HTML_HELP_CLOSE(),

	// 2 Similarity function
	HTML_HELP_OPEN()
		HTML_HELP_DEF("Type", "StringCollection")
		HTML_HELP_DEF("Values", "Reciprocal;Normalized;Exponential")
		HTML_HELP_DEF("Default", "Reciprocal")
		HTML_HELP_BODY()
		"The similarity measure to use."
		HTML_HELP_CLOSE(),

	// 3 Feature property
	HTML_HELP_OPEN()
		HTML_HELP_DEF("Type", "DoubleProperty")
		HTML_HELP_DEF("Default", "viewMetric")
		HTML_HELP_BODY()
		"The DoubleProperty where the similarity will be stored."
		HTML_HELP_CLOSE(),

	// 4 Normalization factor
	HTML_HELP_OPEN()
		HTML_HELP_DEF("Type", "double")
		HTML_HELP_DEF("Default", "1")
		HTML_HELP_BODY()
		"The normalization factor used by the Exponential similarity measure."
		HTML_HELP_CLOSE()
};
}

class DoubleVectorSimilarityComputer: public Algorithm {
private:
	tlp::DoubleVectorProperty *sourceProperty;
	tlp::DoubleProperty       *resultProperty;
	tlp::StringCollection      similarityFunction;
	tlp::StringCollection      distanceFunction;
	double                     normalizationFactor;

	enum distance_t   { Euclidian };
	enum similarity_t { Reciprocal, Normalized, Exponential };
	distance_t        distance;
	similarity_t      similarity;

public:
	PLUGININFORMATIONS(PLUGIN_NAME, "Cyrille Faucheux", "2013-08-09", "", "1.0", "Similarity")

	DoubleVectorSimilarityComputer(PluginContext *context) :
		Algorithm(context)
	{
		addInParameter< tlp::DoubleVectorProperty > ("source", paramHelp[0], "data");
		addInParameter< tlp::StringCollection >     ("distance function", paramHelp[1], "Euclidian");
		addInParameter< tlp::StringCollection >     ("similarity function", paramHelp[2], "Reciprocal;Normalized;Exponential");
		addInParameter< double >                    ("normalization factor", paramHelp[4], "1");
		addInParameter< tlp::DoubleProperty >       ("result", paramHelp[3], "viewMetric");
	}
	~DoubleVectorSimilarityComputer() {}

	bool check(std::string &err)
	{
		try {
			if(dataSet == NULL)
				throw std::runtime_error("No dataset provided.");

			CHECK_PROP_PROVIDED("source",             this->sourceProperty);
			CHECK_PROP_PROVIDED("distance function",   this->distanceFunction);
			CHECK_PROP_PROVIDED("similarity function", this->similarityFunction);
			CHECK_PROP_PROVIDED("result",             this->resultProperty);

			if(distanceFunction.getCurrentString().compare("Euclidian") == 0) {
				distance = Euclidian;
			} else {
				throw std::runtime_error("Unknown distance function.");
			}

			if(similarityFunction.getCurrentString().compare("Reciprocal") == 0) {
				similarity = Reciprocal;
			} else if(similarityFunction.getCurrentString().compare("Normalized") == 0) {
				similarity = Normalized;
			} else if(similarityFunction.getCurrentString().compare("Exponential") == 0) {
				CHECK_PROP_PROVIDED("normalization factor", normalizationFactor);

				if(normalizationFactor <= 0)
					throw new std::runtime_error("The \"normalization factor\" must be strictly positive.");

				normalizationFactor *= normalizationFactor;
				similarity= Exponential;
			} else {
				throw std::runtime_error("Unknown similarity function.");
			}

			// TODO Check if the source has a constant size for each node
		} catch (std::runtime_error &ex) {
			err.assign(ex.what());
			return false;
		}

		return true;
	}

	bool run()
	{
		double d, d_max = 0.0;

		tlp::Iterator< tlp::edge > *itEdges = graph->getEdges();
		while(itEdges->hasNext())
		{
			const tlp::edge e = itEdges->next();
			const tlp::node u = graph->source(e),
			                v = graph->target(e);

			const std::vector< double > fu = this->sourceProperty->getNodeValue(u),
			                            fv = this->sourceProperty->getNodeValue(v);

			if(distance == Euclidian) {
				d = euclidianDistance< std::vector< double > >(fu, fv);
			}

			if(similarity == Reciprocal) {
				resultProperty->setEdgeValue(e, reciprocalSimilarity(d));
			} else if(similarity == Normalized) {
				if(d > d_max)
					d_max = d;
				resultProperty->setEdgeValue(e, d);
			} else if(similarity == Exponential) {
				resultProperty->setEdgeValue(e, exponentialSimilarity(d, normalizationFactor));
			}
		}
		delete itEdges;

		if(similarity == Normalized) {
			if(d_max > 0) {
				itEdges = graph->getEdges();
				while(itEdges->hasNext()) {
					const tlp::edge e = itEdges->next();
					resultProperty->setEdgeValue(e, 1 - resultProperty->getEdgeValue(e) / d_max);
				}
				delete itEdges;
			} else {
				resultProperty->setAllEdgeValue(1.0);
			}
		}

		return true;
	}
};

PLUGIN(DoubleVectorSimilarityComputer);
