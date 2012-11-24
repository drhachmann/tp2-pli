#ifndef _BNC_H_
#define _BNC_H_


#include <cstdlib>
#include <cstring>
#include <iostream>
#include <fstream>
#include <list>
#include <vector>
using namespace std;

#include <ilcplex/ilocplex.h>
ILOSTLBEGIN


enum CutMode{ CLQ2A, CLQ2B };

class BNC{
	public:
		BNC( char*, int, int, char* );
		~BNC();
		int n_cortes;
		int max_deep;
		
		void solve();
	private:
		
		void solveFNBB();
		void solveCLQBB();
		void solveCLQBC();
		void buildModelNF();
		void buildModelCF();
		void printResult();
		void configureCPLEX();
		
		char* mod;
		int time_limit;
		int primal_heuristic;
		
		int m;
		int n;
		
		int** graph;
		vector<vector<int > > adj;
		
		//cplex variables
		IloEnv* env;
		IloModel *model;
		IloNumVarArray *variables;
		IloRangeArray *constraints;
		IloCplex *cplex;
};

#endif
