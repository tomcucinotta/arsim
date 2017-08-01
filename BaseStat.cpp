#include "BaseStat.hpp"
#include "util.hpp"

double BaseStat::getPMFPercentile(double p) const {
  double cdf_value = 0.0;
  for (long n = 0; n < getPMFSize(); ++n) {
    cdf_value += getPMFValue(n);
    if (cdf_value >= p)
      return getXValue(n);
  }
  return getMax();
}

void BaseStat::dumpStat(const char *fname, const char *var_name,
			bool dump_as_pmf, const char *comment)
{
  Logger::debugLog("# Opening output file %s for statistics\n", fname);
  FILE *file = fopen(fname, "w");
  ASSERT(file != 0, "Couldn't open output file for statistics");
  fprintf(file, "# %s ", comment);
  if (dump_as_pmf)
    fprintf(file, "(PMF)\n");
  else
    fprintf(file, "(PDF)\n");
  fprintf(file, "# Min = %g, Max = %g, Mean = %g, SDev = %g\n",
      getMin(), getMax(), getMean(),
      getDev());
  fprintf(file, "# MeanPos = %g, MeanNegZero = %g, MeanAbs = %g\n",
      getMeanPos(), getMeanNegZero(),
      getMeanAbs());
  fprintf(
      file,
      "# Percentiles (00, 05, 10, 15, 85, 90, 95, 98, 99, 99.5, 100) = ( %g %g %g %g %g %g %g %g %g %g %g )\n",
      getMin(), getPMFPercentile(0.05),
      getPMFPercentile(0.10), getPMFPercentile(0.15),
      getPMFPercentile(0.85), getPMFPercentile(0.90),
      getPMFPercentile(0.95), getPMFPercentile(0.98),
      getPMFPercentile(0.99), getPMFPercentile(0.995),
      getMax());
  fprintf(file, "# %9s %11s\n", var_name, "Pr");
  double dx = getXValue(1) - getXValue(0);
  for (long n = 0; n < getPMFSize(); ++n) {
    double x = getXValue(n);
    double p = getPMFValue(n);
    if (!dump_as_pmf)
      p /= dx; /**< Adapt to a PDF-like representation */
    fprintf(file, "%11.5g %11.5g\n", x, p);
  }
  fclose(file);
}
