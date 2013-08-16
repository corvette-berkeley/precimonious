#include "cov_log.h"
#include "cov_checker.h"

int main(int argc, char*argv[]) {
	long double arr[3];
	arr[0] = 0.123451243534534;
	arr[1] = 0.235341232353411;
	arr[2] = 0.398885094850490;
//  cov_log("ideal", "spec.cov", 2, 0.1L, 0.2L);
//  cov_log("delta", "spec.cov", 1, 0.0L);
//	cov_spec_log(0.1, 0, "spec.cov");
//	cov_log(argv[0], "log.cov", 2, 0.1L, 0.2L);
  cov_check("log.cov", "spec.cov", 2);
//	cov_arr_log(arr, 3, argv[0], "log.cov");
//	cov_arr_check("log.cov", "spec.cov", 3);
	return 0;
}
