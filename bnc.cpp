
#include "bnc.h"


int bestIncumbent = -1;
int op1=0;
int op2=0;

int current_deep;
//int max_deep = 1000000;


BNC::BNC( char* mo, int tl, int ph, char* in ){
	mod = mo;
	time_limit = tl;
	primal_heuristic = ph;
	
	ifstream in_file( in, ifstream::in );
	
	if( !in_file.is_open() ){
		cout << "Error in file bnc.cpp, constructor BNC: Could not open file " << in << endl ;
		exit(-1);
	}
	
	{
	char line[50];
	in_file.getline( line, 50 );
	}
	
	in_file >> n >> m;
	printf("DIMENSOES %d %d\n", n, m);
	adj = vector<vector<int > >(n);
	
	graph = new int*[n];
	for( int i = 0; i < n; ++i )
		graph[i] = new int[n];
	
	for( int i = 0; i < n; ++i )
		for( int j = 0; j < n; ++j )
			graph[i][j] = 0;
	
	for( int k = 0; k < m; ++k ){
		int i, j;
		in_file >> i >> j;
		--i; --j;
		adj[i].push_back(j);
		adj[j].push_back(i);
		graph[i][j] = 1;
		graph[j][i] = 1;
	}
	
	env = NULL;
	model = NULL;
	variables = NULL;
	constraints = NULL;
	cplex = NULL;
	
	in_file.close();
	
}

BNC::~BNC(){
	
	for( int i = 0; i < n; ++i )
		delete [] graph[i];
	delete [] graph;
	
	delete cplex;
	delete variables;
	delete constraints;
	delete model;
	delete env;
	
}

//set the current_deep variable to the deep of current node
ILONODECALLBACK1( MySelect, int*, deep ){
	//getEnv().getDepth();
	*deep = getDepth(0);
	
// 	IloInt remainingNodes = getNremainingNodes();
// 	for (IloInt i = 0; i < remainingNodes; i++) {
// 		int depth = getDepth(i);
// 		cout << depth << " ";
// 	}
// 	cout << endl;
}

ILOHEURISTICCALLBACK3(Rounddown, IloNumVarArray, vars, int**, graph, vector<vector<int> >, adj) {
	//if(0){
	int n = vars.getSize()/2;
	
	IntegerFeasibilityArray feas( getEnv() );
	IloNumArray x( getEnv() );
	
	getFeasibilities( feas, vars );
	getValues( x, vars );
	
	int newobj1 = 0;
	int newobj2 = 0;
	int newx1[2*n];
	int newx2[2*n];
	
	memset(newx1, 0, sizeof(newx1));
	memset(newx2, 0, sizeof(newx2));
	int acc[2*n];
	memset(acc, 0, sizeof(acc));
	for(int set=0; set<2; set++){
		for(int i=0; i<n; i++){
			acc[i+n*set]+=x[i+n*set];
			for(int j=0; j<adj[i].size(); j++){
				int v = adj[i][j];
				acc[i]+= x[v+n*set];
			}
		}
	}


	for(int s = 1; s <= 2;s++){
		for( int set = 0; set < 2; ++set ){
			bool marked[n];
			int count_marked = 0;
			for( int i = 0; i < n; ++i ){
				if(feas[i+n*set] == Infeasible)
					marked[i] = false;
				else{
					marked[i]=true;
					count_marked++;
				}
			}

			int winner;
	
			while( count_marked < n ){
				//choose the star node
				winner = -1;
				for( int i = 0; i < n; ++i ){
					if( !marked[i] ){
						if( winner == -1  ){
							winner = i;
						}
						else if(s==1 && x[i+n*set] > x[winner+n*set] ){
							winner = i;
						}else if(s==2 && acc[i+n*set] < acc[winner+n*set])
							winner = i;
					}
				}
			
				marked[winner] = true;
				++count_marked;
				if(set==1 && (s==1&&newx1[winner]==1 || s==2&&newx2[winner]==1)){
					continue;
				}
				if(s==1){
					newx1[winner+n*set] = 1;
					newobj1++;			
				}else{ //s==2
					newx2[winner+n*set] = 1;
					newobj2++;
				}

				for( int i = 0; i < n; ++i ){
					if( graph[i][winner] ){
						if( !marked[i] ){
							marked[i] = true;
							if(s==1){
								newx1[i+set*n] = 0;
							}else{
								newx2[i+set*n] = 0;
							}
							++count_marked;
							marked[winner] = true;
						}
					}
				}
				int temp = 0;
				for( int i = 0; i < n; i++)
					temp+=marked[i];
				if( temp != count_marked ){
					printf("error %d %d\n", temp, count_marked);
					exit(-1);
				}
			
			}
		
		}
	}
	//codigo do incumbent
	//ilomipex4
	
		int *newx;
		int newobj;
		int op=-1;
		if(newobj1 > newobj2){
			newx = newx1;
			newobj = newobj1;
			op=1;
		}else{
			newx = newx2;
			newobj = newobj2;
			op=2;
		}

		if(newobj > bestIncumbent){
			if(op==1)op1++;
			else op2++;
			int cont=0;
			for(int i=0; i<2*n; i++){
				x[i] = newx[i];
				cont += x[i];
			}
			if(cont!=newobj){
				puts("ERRO");
			}
	
			setSolution( vars, x, newobj );
		}
	x.end();
	feas.end();
//	}
}

bool getCut( IloNumArray& vals, IloNumVarArray& vars, CutMode cutmode, int set, 
						IloCplex::ControlCallbackI::IntegerFeasibilityArray feas, 
						IloRange& cut, int** graph, int* old_winner ){

	int n = vals.getSize()/2;
	
	bool marked[n];
	for( int i = 0; i < n; ++i )
		marked[i] = false;
	
	int winner;
	int count_marked = 0;
	list<int> indices;

	int acc[n][n];//mudar para lista adjacencia
	memset(acc, 0, sizeof(acc));
	
	static int cont=0;
	for( int i = 0; i < n; ++i ){
			if(feas[i+n*set] == IloCplex::ControlCallbackI::Infeasible)
				marked[i] = false;
			else{
				marked[i]=true;
				count_marked++;
			}
		}


		//int cont=0;
		int cnt=0;
		while( count_marked < n ){
			//choose the star node
			cnt++;


			winner = -1;
			for( int i = 0; i < n; ++i ){
				if(old_winner[i+n*set])continue;

				if( !marked[i] ){
					if( winner == -1 ){
						winner = i;
					}
					else if( cutmode == CLQ2B && fabs(vals[i+n*set]-0.5) < fabs(vals[winner+n*set]-0.5) && vals[i+n*set] > 0 && vals[i+n*set] < 1 ){
						winner = i;
					}
					else if( cutmode == CLQ2A && vals[i+n*set] > vals[winner+n*set] && vals[i+n*set] < 1 ){
						winner = i;
					}
				}
			}
			if(winner==-1)return false; //??

			old_winner[winner+n*set]=1;
			++count_marked;
			marked[winner] = true;
			indices.push_back( winner+n*set );
		

			for( int i = 0; i < n; ++i ){
				if( !graph[i][winner] ){
					if( !marked[i] ){
						marked[i] = true;
						++count_marked;
					}
				}
			}
		}
	
	list<int>::iterator it = indices.begin();
	float sum = 0;
	while( it != indices.end() ){
		sum += vals[*it];
		++it;
	}
	for(list<int>::iterator it1 = indices.begin(); it1!= indices.end(); it1++){
		for(list<int>::iterator it2 = indices.begin(); it2!= indices.end(); it2++){
			int v1 = *it1;
			int v2 = *it2;
			if(v1==v2)continue;
			v1 -= n*set;
			v2 -= n*set;
			if(!graph[v1][v2] || !graph[v2][v1]){
				puts("ERRO NAO CLICK");
				printf("%d %d\n", v1, v2);
				exit(-1);
			}
		}
	}
	
	if(sum > 1+0.000001 ){
		it = indices.begin();
		while( it != indices.end() ){
			cut.setLinearCoef( vars[*it], 1 );
			//printf("x[%d] ", *it);
			++it;
		}
		//printf(" <= 1\n");
		return true;
	}
	return false;
}


ILOUSERCUTCALLBACK4( CtCallback, IloNumVarArray, vars, int**, graph, int, num_cuts, int, max_deep ) {
	
	if( current_deep > max_deep ){
		//cout << "aa " << current_deep << endl;
		return;
	}
	
	IloNumArray vals(getEnv());
	getValues(vals, vars);
	IntegerFeasibilityArray feas( getEnv() );
	getFeasibilities( feas, vars );
	int old_win[vals.getSize()];
	memset(old_win, 0, sizeof(old_win));
	for(int n=0;  n<num_cuts; n++){
		static int cont=0;
		int cnt=0;
		for( int i = 0; i < 2; ++i ){
			IloRange cut( getEnv(), 0, 1 );
			if( getCut( vals, vars, CLQ2B, i, feas, cut, graph, old_win ) ){
				add(cut);
				cnt++;
				
			}
			else if( getCut( vals, vars, CLQ2A, i, feas, cut, graph, old_win ) ){
				cnt++;
				add(cut);
			}else{ //se nao encotrar algum corte, sai
				
								
			}
			cut.end();
		}
		if(n>1 && cnt==0)break;
	
			
	}
	feas.end();
	vals.end();
}


void BNC::buildModelNF(){
	env = new IloEnv;
	model = new IloModel(*env);
	variables = new IloNumVarArray(*env);
	constraints = new IloRangeArray(*env);
	
	//create variables
	for( int i = 0; i < n; ++i ){
		variables->add( IloIntVar( *env, 0, 1 ) );
		variables->add( IloIntVar( *env, 0, 1 ) );
	}
	
	
	for( int i = 0, k = 0; i < n; ++i ){
		for( int j = i; j < n; ++j ){
			if( graph[i][j] ){
				IloRange newconstraintA( *env, 0, 1 );
				IloRange newconstraintB( *env, 0, 1 );
				
				newconstraintA.setLinearCoef( (*variables)[i], 1 );
				newconstraintA.setLinearCoef( (*variables)[j], 1 );
				
				newconstraintB.setLinearCoef( (*variables)[n+i], 1 );
				newconstraintB.setLinearCoef( (*variables)[n+j], 1 );
				
				constraints->add( newconstraintA );
				constraints->add( newconstraintB );
				++k;
			}
		}
	}
	
	for( int i = 0; i < n; ++i ){
		IloRange newconstraintAB( *env, 0, 1 );
		newconstraintAB.setLinearCoef( (*variables)[i], 1 );
		newconstraintAB.setLinearCoef( (*variables)[n+i], 1 );
		constraints->add( newconstraintAB );
	}
		
	//objective function
	IloObjective obj = IloMaximize(*env);
	
	//objective
	for( int i = 0 ; i < n; i++ ){
		obj.setLinearCoef((*variables)[i], 1 );
		obj.setLinearCoef((*variables)[n+i], 1 );
	}
	
	model->add( *constraints );
	model->add( obj );
	
	cplex = new IloCplex(*model);
	
	configureCPLEX();
	
	//write model in file cplexmodel.lp
	cplex->exportModel("cplexmodel.lp");
}

void BNC::buildModelCF(){
	env = new IloEnv;
	model = new IloModel(*env);
	variables = new IloNumVarArray(*env);
	constraints = new IloRangeArray(*env);
	
	list< list<int> > cliques;
	
	
	int count_chose = 0;
	
	bool chose_edge[n][n];
	for( int i = 0; i < n; ++i )
		for( int j = 0; j < n; ++j )
			chose_edge[i][j] = false;
	
	while( count_chose < m ){
		list<int> current_clique;
		
		//choose a initial node
		for( int i = 0; i < n; ++i ){
			for( int j = 0; j < n; ++j ){
				if( graph[i][j] ){
					if( !chose_edge[i][j] ){
						chose_edge[i][j] = chose_edge[j][i] = true;
						++count_chose;
						current_clique.push_back(i);
						current_clique.push_back(j);
						goto done;
					}
				}
			}
		}
		done:
		//build a clique
		int i = current_clique.front();
		for( int j = 0; j < n; ++j ){
			if( graph[i][j] ){
				
				if( !chose_edge[i][j] ){
					bool add_node = true;
					
					list<int>::iterator it = current_clique.begin();
					while( it != current_clique.end() ){
						if( !graph[*it][j] ){
							add_node = false;
							break;
						}
						++it;
					}
					
					if( add_node ){
						{
						list<int>::iterator it = current_clique.begin();
						while( it != current_clique.end() ){
							if( !chose_edge[*it][j] )
								++count_chose;
							
							chose_edge[*it][j] = chose_edge[j][*it] = true;
							++it;
						}
						}
						current_clique.push_back(j);
					}
				}
			}
		}
		
		cliques.push_back( current_clique );
	}
	
	//create variables
	for( int i = 0; i < n; ++i ){
		variables->add( IloIntVar( *env, 0, 1 ) );
		variables->add( IloIntVar( *env, 0, 1 ) );
	}
	
	list< list<int> >::iterator it1 = cliques.begin();
	while( it1 !=  cliques.end() ){
		list<int>::iterator it2 = it1->begin();
		IloRange newconstraintA( *env, 0, 1 );
		IloRange newconstraintB( *env, 0, 1 );
		while( it2 != it1->end() ){
			newconstraintA.setLinearCoef( (*variables)[*it2], 1 );
			newconstraintB.setLinearCoef( (*variables)[n+*it2], 1 );
			++it2;
		}
		constraints->add( newconstraintA );
		constraints->add( newconstraintB );
		++it1;
	}

	for( int i = 0; i < n; ++i ){
		IloRange newconstraintAB( *env, 0, 1 );
		newconstraintAB.setLinearCoef( (*variables)[i], 1 );
		newconstraintAB.setLinearCoef( (*variables)[n+i], 1 );
		constraints->add( newconstraintAB );
	}
		
	//objective function
	IloObjective obj = IloMaximize(*env);
	
	//objective
	for( int i = 0 ; i < n; i++ ){
		obj.setLinearCoef((*variables)[i], 1 );
		obj.setLinearCoef((*variables)[n+i], 1 );
	}
	
	model->add( *constraints );
	model->add( obj );
	
	cplex = new IloCplex(*model);
	
	configureCPLEX();
	
	//write model in file cplexmodel.lp
	cplex->exportModel("cplexmodel.lp");
}

void BNC::solve(){
	if( !strcmp( mod, "FN" ) ){
		solveFNBB();
	}
	else if( !strcmp( mod, "CLQBB" ) ){
		solveCLQBB();
	}
	else if( !strcmp( mod, "CLQBC" ) ){
		solveCLQBC();
	}
	else{
		cout << "Error in file bnc.cpp, function solve: Undefined model" << endl;
		exit(-1);
	}
}

//implementation of the solver by natural formulation + b&b
void BNC::solveFNBB(){
	
	buildModelNF();
	
	if( !cplex->solve() ){
		env->error() << "Failed to optimize LP" << endl;
		throw(-1);
	}
	
	printResult();
	
}

//implementation of the solver by CLQ model + b&b
void BNC::solveCLQBB(){
	
	buildModelCF();
	
	if( !cplex->solve() ){
		env->error() << "Failed to optimize LP" << endl;
		throw(-1);
	}
	
	printResult();
}

//implementation of the solver by CLQ model + b&c
void BNC::solveCLQBC(){
	
	buildModelCF();
	
	cplex->use( CtCallback(*env, *variables, graph, n_cortes, max_deep ) );
	try{
		if( !cplex->solve() ){
			env->error() << "Failed to optimize LP" << endl;
			throw(-1);
		}
		}catch (IloException &e){
			env->error() << e.getMessage();
		}
	printResult();
}

void BNC::configureCPLEX(){

	op1=op2=0;

	//disable output
	//cplex->setOut(env->getNullStream());
		
	//define a time limit execution

	cplex->setParam( IloCplex::TiLim, time_limit );
	//disable presolve
	cplex->setParam( IloCplex::PreInd, false );
	
	//assure linear mappings between the presolved and original models
	cplex->setParam( IloCplex::PreLinear, 0 );
	
	//Turn on traditional search for use with control callback
	cplex->setParam( IloCplex::MIPSearch, IloCplex::Traditional);
	
	
	//Decides how often to apply the periodic heuristic. Setting the value to -1 turns off the periodic heuristic
	cplex->setParam( IloCplex::HeurFreq, 0 );

	
	//CPX_PARAM_MIPCBREDLP equivalent is not availible in c++ api
	//cpx_ret = CPXsetintparam (env, CPX_PARAM_MIPCBREDLP, CPX_OFF);
	
	/* impressao para conferencia */
	if (primal_heuristic) {
		//cout << "*** Primal Heuristic is going to be used." << endl;
		cplex->use(Rounddown(*env, *variables, graph, adj));
	}
	
	cplex->setParam( IloCplex::FracCuts, -1 );

	//disable cplex cutting separation
	cplex->setParam( IloCplex::CutsFactor, 1.0);//conferir valor

	//Decides whether or not Gomory fractional cuts should be generated for the problem: -1 disables cuts
	cplex->setParam( IloCplex::FracCuts, -1 );

	//Controls whether CPLEX applies a local branching heuristic to try to improve new incumbents found during a MIP search
	cplex->setParam( IloCplex::LBHeur, false );

	//Set the upper limit on the number of cutting plane passes CPLEX performs when solving the root node of a MIP model
	printf("N cortes %d\n", n_cortes);
	printf("MaxDeep %d\n", max_deep);
	

	//status = CPXsetintparam (env, CPX_PARAM_DATACHECK, CPX_ON);
	
	//actives the node callback MySelect
	cplex->use( MySelect( *env, &current_deep ) );
}

void BNC::printResult(){

	IloNumArray vals(*env);
	env->out() << "Solution status = " << cplex->getStatus() << endl;
	env->out() << "Solution value  = " << cplex->getObjValue() << endl;
	env->out() << "GAP             = " << cplex->getMIPRelativeGap()*100 << "%"<< endl;
	
	cout << "Heuristica 1 : " <<op1 << endl;
	cout << "Heuristica 2 : " <<op2 << endl;
	cplex->getValues(vals, *variables);
	cout << "A = [ ";
	for( int i = 0; i < n; ++i )
		cout << vals[i] << " ";
	cout << "]" << endl;
	cout << "B = [ ";
	for( int i = 0; i < n; ++i )
		cout << vals[n+i] << " ";
	cout << "]" << endl;
}
