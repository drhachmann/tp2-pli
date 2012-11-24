#include "bnc.h"

int main( int argc, char** argv ){
	char* model;
	int time_limit;
	int primal_heuristic;
	char* input_file;
	int nc; //number of cuts
	int md; //maxdeep
	
	model = argv[1];
	sscanf( argv[2], "%d", &time_limit );
	sscanf( argv[3], "%d", &primal_heuristic );
	sscanf( argv[5], "%d", &nc);
	sscanf( argv[6], "%d", &md);
	
	input_file = argv[4];
	printf("LINE %d\n", __LINE__);
	BNC bnc( model, time_limit, primal_heuristic, input_file );
	
	bnc.n_cortes = nc;
	bnc.max_deep = md;
	
	bnc.solve();
	return (0);
}
