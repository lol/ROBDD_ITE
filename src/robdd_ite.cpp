////////////////////////////////////////////////////////////////////////
// File: BDD Package for Levelized Circuits
// ROBDD using ite()
//
// *** Usage ***
// Usage: ./a.out <ckt> <vector>
////////////////////////////////////////////////////////////////////////

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <vector>
#include <fstream>
//#include <algorithm>
#include <set>
#include <map>
#include <unordered_map>
#include <sstream>

#define NUM_BUCKETS 10000

class node
{

  // A = id, var, low, high, inv = something. not(A) = id, var, low, high, inv = !something.
  // Such nodes will have same id. ^ Only inv will be used to differentiate between them.
  // Inherently different nodes have different id.
public:
  int id;
  int var;
  node *low;
  node *high;
  node *next;
  bool inv;

  node()
  {
    id = 0;
    var = 0;
    low = NULL;
    high = NULL;
    next = NULL;
    inv = false;
  }

  node(int var_, bool inv_, node* high_, node* low_, int id_)
  {
    id = id_;
    var = var_;
    inv = inv_;
    high = high_;
    low = low_;
    next = NULL;
  }

  void printNode()
  {
    std::cout << "id = " << id << ", var = " << var;
    std::cout << ", high = ";
    if(high == NULL)
      std::cout << "NULL";
    else
      std::cout << high->id;
    std::cout << ", low = ";
    if(low == NULL)
      std::cout << "NULL";
    else
      std::cout << low->id;
    std::cout << std::endl;
  }

  void printNodeClean()
  {
    std::cout << id << " " << var << " ";
    if(high == NULL)
      std::cout << "0 ";
    else
      std::cout << high->id << " ";
    if(low == NULL)
      std::cout << "0 ";
    else
      std::cout << low->id << " ";
    std::cout << std::endl;
  }

  void printNodeWithInv()
  {
    std::cout << id << " " << var << " ";
    if(high == NULL)
      std::cout << "0 ";
    else
      std::cout << high->id << " ";
    if(low == NULL)
      std::cout << "0 ";
    else
      std::cout << low->id << " ";
    if(inv)
      std::cout << "INVERTED";
    std::cout << std::endl;
  }

  /*
    bool operator==(const node& other) const
    {
    return (id == other.id && var == other.var && low == other.low && high == other.high && inv == other.inv);

    // need not check next
    }
  */
    
};

class hashTable
{
public:
  node** htable;
  int size;

  hashTable(int n)
  {
    size = n;
    htable = new node*[size];
    for(int i = 0; i < size; ++i)
      htable[i] = NULL;
  }
  
  ~hashTable()
  {
    for(int i = 0; i < size; ++i)
      {
	node* entry = htable[i];
	while(entry != NULL)
	  {
	    node* prev = entry;
	    entry = entry->next;
	    delete prev;
	  }
      }
    delete[] htable;
  }

  inline int hash(int x, int y, int z)
  {
    return (x + y + z) % NUM_BUCKETS;
  }

  // find(), and add accordingly.
  node* insert(int var, bool inv, node* high, node* low, int &idCount)
  {
    bool partialFind = false;
    int partialFindId = 0;
    node* curr = find(var, inv, high, low, partialFind, partialFindId);

    if(curr != NULL)
      return curr;
    else
      {
	int hash_val = hash(var, high->id, low->id);

	if(htable[hash_val] == NULL)
	  {
	    if(partialFind == false)
	      {
		++idCount;
		htable[hash_val] = new node(var, inv, high, low, idCount);
	      }
	    else
	      {
		//++idCount;
		htable[hash_val] = new node(var, inv, high, low, partialFindId);
	      }
	    
	    return htable[hash_val];
	  }
	else
	  {
	    node* curr = htable[hash_val];

	    while(curr != NULL && curr->next != NULL)
	      {
		curr = curr->next;
	      }

	    if(partialFind == false)
	      {
		++idCount;
		curr->next = new node(var, inv, high, low, idCount);
	      }
	    else
	      {
		curr->next = new node(var, inv, high, low, partialFindId);
	      }

	    return curr->next;
	  }
      }
   
  }
	
	

  // Finds a node. partialFind is set true if inverted node is already in the uniqueTable. partialFindId is set to that id.
  node* find(int var, bool inv, node* high, node* low, bool &partialFind, int &partialFindId)
  {
    int hash_val = hash(var, high->id, low->id);

    if(htable[hash_val] == NULL)
      {
	return NULL;
      }
    else
      {
	node* curr = htable[hash_val];

	while(curr != NULL)
	  {
	    if(curr->var == var && curr->high == high && curr->low == low)
	      {
		partialFind = true;
		if(partialFind)
		  partialFindId = curr->id;
		//std::cout << "\n" << partialFindId << "\n";
		if(curr->inv == inv)
		  return curr;
	      }
	    curr = curr->next;
	  }
	return NULL;
      }
  }

  void printTable()
  {
    for(int i = 0; i < size; ++i)
      {
	std::cout << "Bucket " << i << ": ";
	node* curr = htable[i];
	while(curr != NULL)
	  {
	    curr->inv ? std::cout << "-" : std::cout << "";
	    std::cout << curr->id << " " << curr->var << " ";
	    
	    curr->high->inv ? std::cout << "-" : std::cout << "";
	    std::cout << curr->high->id << " ";
	    
	    curr->low->inv ? std::cout << "-" : std::cout << "";
	    std::cout << curr->low->id;
	    
	    std::cout << "; ";
	    
	    curr = curr->next;
	  }
	std::cout << "\n";
      }
    std::cout << "\n\n";
  }

  
};


// For memoization. If ite(f, g, h) has been called, return the already computed node. This prevents recomputation and saves time.
class iteCall
{
public:
  node* f;
  node* g;
  node* h;

  iteCall()
  {
    f = NULL;
    g = NULL;
    h = NULL;
  }

  iteCall(node* i, node* t, node* e)
  {
    f = i;
    g = t;
    h = e;
  }

  //Must define Equality operator to use custom classes with unordered_map, map etc. Enables definition of hash<iteCall>
  bool operator==(const iteCall& other) const
  {
    return (f == other.f && g == other.g && h == other.h);
  }
};

// define hash<iteCall> to enable use of iteCall with unordered_map, map etc.
namespace std {
  template<>
  class hash<iteCall>
  {
  public:
    std::size_t operator()(const iteCall& k) const
    {
      using std::size_t;
      using std::hash;

      return ((hash<node*>()(k.f) ^ (hash<node*>()(k.g) << 1)) >> 1) ^ (hash<node*>()(k.h) << 1);
    }
  };
}

// Gate type enumeration in .lev file.
enum
  {
    G_JUNK,         /* 0 */
    G_INPUT,        /* 1 */
    G_OUTPUT,       /* 2 */
    G_XOR,          /* 3 */
    G_XNOR,         /* 4 */
    G_DFF,          /* 5 */
    G_AND,          /* 6 */
    G_NAND,         /* 7 */
    G_OR,           /* 8 */
    G_NOR,          /* 9 */
    G_NOT,          /* 10 */
    G_BUF,          /* 11 */
    G_TIE1,         /* 12 */
    G_TIE0,         /* 13 */
  };

////////////////////////////////////////////////////////////////////////
// circuit class
////////////////////////////////////////////////////////////////////////

class circuit
{
  // circuit information
  int numgates;       // total number of gates (faulty included)
  int maxlevels;      // number of levels in gate level ckt
  int numInputs;	// number of inputs
  int numOutputs;	// number of outputs
  unsigned char *gtype;	// gate type
  short *fanin;		// number of fanins of gate
  short *fanout;		// number of fanouts of gate
  int *levelNum;		// level number of gate
  int **faninlist;		// fanin list of gate
  int **fanoutlist;		// fanout list of gate

  // you may wish to add more private variables here
  int numNodes;         // number of nodes - dfs() used for counting
  std::vector<int> outputs;

  std::vector<int> order;
  std::vector<node*> gateBDD;
  hashTable *uniqueTable;
  std::unordered_map<iteCall, node*> computedTable;
  int idCount;
  std::unordered_map<int, std::vector<int> > unordered;
  node* _0, *_1;
  std::set<node *> visitedNodes;

public:
  circuit(char *);            // constructor
  void dispGateInfo(int);
  void dfs(node*);
  node* posFactor(node*, int);
  node* negFactor(node*, int);
  int topVar(node*, node*, node*);
  node* ite(node*, node*, node*);
  node* _not(node*);
  void simulate(std::string);
  void levelizedBuild();
  void setOrder();
  void setOrder(std::vector<int>);
  void print(std::string);
  int getNumInputs();

  // you may wish to add more function declarations (or public variables) here
};

////////////////////////////////////////////////////////////////////////
// constructor: reads in the *.lev file and builds basic data structure
// 		for the gate-level ckt
////////////////////////////////////////////////////////////////////////
circuit::circuit(char *cktName)
{
  FILE *inFile;
  char fName[40];
  int i, j, count;
  char c;
  int gatenum, junk;
  int f1, f2, f3;

  strcpy(fName, cktName);
  strcat(fName, ".lev");
  inFile = fopen(fName, "r");
  if (inFile == NULL)
    {
      fprintf(stderr, "Can't open .lev file\n");
      exit(-1);
    }

  numgates = maxlevels = numInputs = numOutputs = 0;

  fscanf(inFile, "%d", &count);	// number of gates
  fscanf(inFile, "%d", &junk);	// skip the second line

  // allocate space for gates data structure
  gtype = new unsigned char[count];
  fanin = new short[count];
  fanout = new short[count];
  levelNum = new int[count];
  faninlist = new int * [count];
  fanoutlist = new int * [count];

  // now read in the circuit
  for (i=1; i<count; i++)
    {
      fscanf(inFile, "%d", &gatenum);
      fscanf(inFile, "%d", &f1);
      fscanf(inFile, "%d", &f2);
      fscanf(inFile, "%d", &f3);

      numgates++;
      gtype[gatenum] = (unsigned char) f1;
      if (gtype[gatenum] > 13)
	printf("gate %d is an unimplemented gate type\n", gatenum);
      else if (gtype[gatenum] == G_INPUT)
	numInputs++;
      else if (gtype[gatenum] == G_OUTPUT)
	{
	  outputs.push_back(gatenum);
	  numOutputs++;
	}

      f2 = (int) f2;
      levelNum[gatenum] = f2;

      if (f2 >= (maxlevels))
	maxlevels = f2 + 5;

      fanin[gatenum] = (int) f3;
      // now read in the faninlist
      faninlist[gatenum] = new int[fanin[gatenum]];
      for (j=0; j<fanin[gatenum]; j++)
	{
	  fscanf(inFile, "%d", &f1);
	  faninlist[gatenum][j] = (int) f1;
	}

      // CAN SKIP THE RESET OF THE LINE IF DESIRED
      for (j=0; j<fanin[gatenum]; j++) // followed by samethings
	fscanf(inFile, "%d", &junk);

      // read in the fanout list
      fscanf(inFile, "%d", &f1);
      fanout[gatenum] = (int) f1;

      fanoutlist[gatenum] = new int[fanout[gatenum]];
      for (j=0; j<fanout[gatenum]; j++)
	{
	  fscanf(inFile, "%d", &f1);
	  fanoutlist[gatenum][j] = (int) f1;
	}

      // skip till end of line
      while ((c = getc(inFile)) != '\n' && c != EOF)
	;
    }	// for (i...) 
  fclose(inFile);

  /*
    printf("Successfully read in circuit:\n");
    printf("\t%d PIs.\n", numInputs);
    printf("\t%d POs.\n", numOutputs);
    printf("\t%d total number of gates\n", numgates);
    printf("\t%d levels in the circuit.\n", maxlevels / 5);
  */

  uniqueTable = new hashTable(NUM_BUCKETS);
  idCount = 3;

  _0 = new node(numInputs + 1, false, NULL, NULL, 1);
  _1 = new node(numInputs + 1, false, NULL, NULL, 2);

  gateBDD.resize(count);

}

// to display details about a gate or port
void circuit::dispGateInfo(int x)
{
  // x is Gate / Port #
  std::cout << "\n";
  std::cout << "Number of FanIns of gate # "<< x << " : " << fanin[x] << "\n";
  std::cout << "Number of FanOuts of gate # "<< x << " : " << fanout[x] << "\n";
  std::cout << "FanIn list of gate # " << x << " : ";
  for(int i = 0; i < fanin[x]; ++i)
    std::cout << faninlist[x][i] << " ";
  std::cout << "\n";
  std::cout << "FanOut list of gate # " << x << " : ";
  for(int i = 0; i < fanout[x]; ++i)
    std::cout << fanoutlist[x][i] << " ";
  std::cout << "\n";
}

// return number of inputs
int circuit::getNumInputs()
{
  return numInputs;
}

// set order to the input variable Order
void circuit::setOrder(std::vector<int> f)
{
  order = f;
}

// default variable order - 1 to numInputs
void circuit::setOrder()
{
  order.push_back(-1);
  for(int i = 1; i <= numInputs; i++)
    order.push_back(i);
  order.push_back(numInputs + 1);  
}

// depth first search from a given node. Call it from all output computed nodes to return the total number of nodes in the computer ROBDD.
void circuit::dfs(node* f)
{
  //std::cout<< f->id << " " << f->var << " " << (f->high->inv ? "-" : "") << f->high->id << " " << (f->low->inv ? "-" : "") << f->low->id << std::endl;

  // for ID renaming
  unordered.insert( { f->id, { f->var, f->high->inv, f->high->id, f->low->inv, f->low->id } } );

  // set of visited nodes
  visitedNodes.insert(f);

  //f->printNode();
  //std::cout << "dfs of high" << std::endl;
  if(!(f->high == _1 || f->high == _0))
    if(visitedNodes.find(f->high) == visitedNodes.end())
      dfs(f->high);

  //f->printNode();
  //std::cout << "dfs of low" << std::endl;
  if(!(f->low == _1 || f->low == _0))
    if(visitedNodes.find(f->low) == visitedNodes.end())
      dfs(f->low);  
   
}

// invert a node
node* circuit::_not(node* f)
{
  //std::cout << "I'm in _not()\n";
  if(f == _1)
    return _0;
  else if(f == _0 )
    return _1;
  else
    return uniqueTable->insert(f->var, !f->inv, f->high, f->low, idCount);
}

// positive edge cofactor wrt x
node* circuit::posFactor(node* f, int x)
{
  node* posChild = f->high;
  
  if(x == f->var && f->inv == false)
    return posChild;
  else if(x == f->var && f->inv == true)
    return _not(posChild);
  else
    return f;
}

// negative edge cofactor wrt x
node* circuit::negFactor(node* f, int x)
{
  node* negChild = f->low;

  if(x == f->var && f->inv == false)
    return negChild;
  else if(x == f->var && f->inv == true)
    return _not(negChild);
  else
    return f;
}

// highest ordered variable in the current computation. Need to cofactor wrt this variable
int circuit::topVar(node* f, node* g, node* h)
{
  //std::cout << "f-var, g-var, h-var = " << f->var << " " << g->var << " " << h->var;
  //std::cout << std::endl;
  if(order[f->var] <= order[g->var] && order[f->var] <= order[h->var])
    return f->var;
  else if(order[g->var] <= order[f->var] && order[g->var] <= order[h->var])
    return g->var;
  else
    return h->var;
}

// compute BDD for every gate in the circuit
void circuit::levelizedBuild()
{
  node *op1, *op2, *eval;

  for(int i = 1; i <= numgates; ++i)
    {
      //std::cout << "i = " << i << " "; // << "\n";
      switch(gtype[i])
	{
	  //Input Port
	case G_INPUT:
	  //std::cout << "Input\n";
	  op1 = uniqueTable->insert(i, false, _1, _0, idCount);
	  gateBDD[i] = op1;
	  //uniqueTable->printTable();
	  break;

	  //Output Port
	case G_OUTPUT:
	  //std::cout << "Output\n";
	  gateBDD[i] = gateBDD[ faninlist[i][0] ];
	  break;

	  //XOR gate
	case G_XOR:
	  //std::cout << "XOR\n";
	  op1 = gateBDD[ faninlist[i][0] ];
	  for(int j = 1; j < fanin[i]; ++j)
	    {
	      op2 = gateBDD[ faninlist[i][j] ];
	      eval = ite(op1, _not(op2), op2);

	      op1 = eval;
	    }
	  gateBDD[i] = eval;
	  break;

	  //XNOR gate
	case G_XNOR:
	  //std::cout << "XNOR\n";
	  for(int j = 1; j < fanin[i]; ++j)
	    {
	      op2 = gateBDD[ faninlist[i][j] ];
	      eval = ite(op1, _not(op2), op2);

	      op1 = eval;
	    }
	  eval = _not(eval);
	  gateBDD[i] = eval;
	  break;

	  //AND gate	  
	case G_AND :
	  //std::cout << "AND\n";
	  op1 = gateBDD[ faninlist[i][0] ];
	  //op1->printNode();
	  for(int j = 1; j < fanin[i]; ++j)
	    {
	      op2 = gateBDD[ faninlist[i][j] ];
	      eval = ite(op1, op2, _0);

	      op1 = eval;
	    }
	  gateBDD[i] = eval;
	  break;

	  //NAND gate
	case G_NAND:
	  //std::cout << "NAND\n";
	  op1 = gateBDD[ faninlist[i][0] ];
	  //op1->printNode();
	  for(int j = 1; j < fanin[i]; ++j)
	    {
	      op2 = gateBDD[ faninlist[i][j] ];
	      eval = ite(op1, op2, _0);

	      op1 = eval;
	    }
	  eval = _not(eval);
	  gateBDD[i] = eval;
	  break;

	  //OR gate
	case G_OR:
	  //std::cout << "OR\n";
	  op1 = gateBDD[ faninlist[i][0] ];
	  for(int j = 1; j < fanin[i]; ++j)
	    {
	      op2 = gateBDD[ faninlist[i][j] ];
	      eval = ite(op1, _1, op2);

	      op1 = eval;
	    }
	  gateBDD[i] = eval;
	  break;

	  //NOR gate
	case G_NOR:
	  //std::cout << "NOR\n";
	  op1 = gateBDD[ faninlist[i][0] ];
	  for(int j = 1; j < fanin[i]; ++j)
	    {
	      op2 = gateBDD[ faninlist[i][j] ];
	      eval = ite(op1, _1, op2);

	      op1 = eval;
	    }
	  eval = _not(eval);
	  gateBDD[i] = eval;
	  break;

	  //NOT gate
	case G_NOT:
	  //std::cout << "NOT\n";
	  op1 = gateBDD[ faninlist[i][0] ];
	  eval = _not(op1);
	  gateBDD[i] = eval;
	  break;
	  
	default:
	  //std::cout << (int)gtype[i] << " DEFAULT\n";
	  gateBDD[i] = gateBDD[ faninlist[i][0] ];
	  break;
	}
      //std::cout<< gateBDD[i]->id << " " << gateBDD[i]->var << " " << (gateBDD[i]->high->inv ? "-" : "") << gateBDD[i]->high->id << " " << (gateBDD[i]->low->inv ? "-" : "") << gateBDD[i]->low->id << std::endl << std::endl;
	 
    }
  std::cout << std::endl;
}

// ITE function
// 1) checks for simplification
// 2) checks whether current call has already been computed
// 3) checks for terminal cases
// 4) recursive call
// 5) reduction
// 6) add to unique table, add call to computed table
node* circuit::ite(node* f, node* g, node* h)
{
  // a gets computed in this call
  node* a;

  // std::cout << "ite(f, g, h) = " << (f->inv ? "-" : "") << f->id << " " << (g->inv ? "-" : "") << g->id << " " << (h->inv ? "-" : "") << h->id << std::endl;

  //ff + f'h
  if(f == g)
    g = _1;
  //ff' + f'h
  if(f == _not(g))
    g = _0;
  //fg + f'f
  if(f == h)
    h = _0;
  //fg + f'f'
  if(f == _not(h))
    h = _1;

  // already computed ite(f, g, h)? Yes, then return the previous computation.
  if(computedTable.end() != computedTable.find( {f, g, h} ))
    {
      //std::cout << " found in computedTable " << std::endl;
      a = computedTable.at( {f, g, h} );
      return a;
    }
 
  /*
    std::cout << "I reached ite()\n";
    std::cout << "----> f = ";
    f->printNode();
    std::cout << "----> g = ";
    g->printNode();
    std::cout << "----> h = ";
    h->printNode();
  */

  //terminal cases
  if(f == _1)
    return g;
  if(f == _0)
    return h;
  if(g == _1 && h == _0)
    return f;
  if(g == _0 && h == _1)
    return _not(f);	
  if(g == h)
    return g;

  int x = topVar(f, g, h);
  //std::cout << "topVar = " << x;
  //std::cout << std::endl;

  //compute the node
  node* r_high = ite(posFactor(f, x), posFactor(g, x), posFactor(h, x));
  node* r_low = ite(negFactor(f, x), negFactor(g, x), negFactor(h, x));

  //reduce
  if(r_low == r_high)
    return r_low;
		
  //uniqueTable->printTable();

  //add to uniqueTable and computedTable
  a = uniqueTable->insert(x, false, r_high, r_low, idCount);
  
  computedTable.insert( { {f, g, h}, a } );

  //std::cout << "Exiting ite()\n";

  return a;
}

// simulate function - follows the tree from the output nodes according to the input vector
void circuit::simulate(std::string vecFileName)
{
  //std::vector<std::string>  vecStr = {"00001", "00110", "10111", "01011", "11101", "11000", "11110", "10010"};

  std::string simFileName = vecFileName + ".sim";
  std::stringstream simOut;

  vecFileName += ".vec";
  
  std::string line;
  std::ifstream vecFile(vecFileName.c_str());
  std::vector<std::string> vecStr;

  // if File not found
  if(!vecFile.good())
    {
      std::cout << vecFileName << " not found.\n";
      exit(-1);
    }


  // ignore the first line
  std::getline(vecFile, line);
  //std::cout << line << "\n";

  // create vector of strings
  while(vecFile.good())
    {
      std::getline(vecFile, line);
      if(line[0] != 'E')
	{
	  //std::cout << line << "\n";
	  if(vecFile.good())
	    {
	      vecStr.push_back(line);
	    }
	}
    }
    
  
  
  std::vector<int> vec;

  int invCount;

  node* curr, * prev;
  //node* curr = gateBDD[ outputs[1] ];
  //  node* prev;

  //curr->printNodeWithInv();

  for(unsigned int j = 0; j < vecStr.size(); ++j)
    {
      for(unsigned int k = 0; k < vecStr[j].size(); ++k)
	{
	  vec.push_back(vecStr[j][k] - '0');
	}

      // compute for every output BDD node
      for(int i = 0; i < numOutputs; ++i)
	{
	  // start simulation from the BDD node computed from this output
	  curr = gateBDD[ outputs[i] ];

	  invCount = 0;

	  //std::cout << "I am here" << std::endl;

	  while(curr != NULL)
	    {
	      prev = curr;

	      //std::cout << "curr = "; curr->printNodeWithInv();

	      //if(curr->low == NULL && curr->high  == NULL)
	      //	break;
	      //
	      //	std::cout << "curr's low = NULL\n";
	      //if(curr->high == NULL)
	      //	std::cout << "curr's high = NULL\n";

	      

	      if(curr->inv == true)
		++invCount;

	      if(curr->low == NULL && curr->high == NULL)
		{
		  curr = NULL;
		}
	      else if(vec[-1 + curr->var] == 1)
		{
		  curr = curr->high;
		  //std::cout << " setting curr to curr->high...\n";
		}
	      else if(vec[-1 + curr->var] == 0)
		{
		  curr = curr->low;
		  //std::cout << "setting curr to curr->low...\n";
		}
	      //std::cout << "prev = "; prev->printNodeWithInv();
	    }

	  // print result
	  if(invCount % 2 == 1)
	    {
	      if(prev == _1)
		simOut << "0";
	      else
		simOut << "1";
	    }
	  else
	    {
	      if(prev == _1)
		simOut << "1";
	      else
		simOut << "0";
	    }
	  
	  //std::cout << curr->id << "\n";
	  
	}
      vec.clear();
      simOut << std::endl;
    }

  std::ofstream outFile;
  outFile.open(simFileName);
  outFile << simOut.rdbuf();
}

// masks the internal node IDs and prints compacted ID numbers
void circuit::print(std::string bddFileName)
{
  //   for(int i = 0; i < ckt->numOutputs; ++i)
  // {
  //  std::cout << (ckt->gateBDD[ ckt->outputs[i] ]->inv ? "-" : "") << ckt->gateBDD[ ckt->outputs[i] ]->id << " ";	
  //  }

  bddFileName += ".bdd";
  std::stringstream bddOut;
  
  for(int i = 0; i < numOutputs; i++)
    {
      dfs(gateBDD[ outputs[i] ]);
    }


  // visitedNodes = set of all nodes connected with the output nodes. Gets updated by dfs().
  numNodes = visitedNodes.size();
  
  std::map<int, std::vector<int> > ordered(unordered.begin(), unordered.end());
  std::unordered_map<int, int> renameIdx;
  int idx = 3;
  renameIdx.insert( { 1, 1} );
  renameIdx.insert( { 2, 2} );
  for(auto it = ordered.begin(); it != ordered.end(); ++it)
    {
      //std::cout << it->first << " " << it->second[0] << " " << (it->second[1] ? "-" : "") << it->second[2] << " " << (it->second[3] ? "-" : "") << it->second[4];
      //std::cout << "\n";
      //std::cout << it ->first << " " << idx << "\n";
      renameIdx.insert( { it->first, idx++ } );
    }



  std::map<int, std::vector<int>> renamed;

  for(auto it = ordered.begin(); it != ordered.end(); ++it)
    {
      renamed.insert( std::pair<int, std::vector<int>> ( renameIdx.at( { it->first } ), { it->second[0], it->second[1], renameIdx.at( { it->second[2] } ), it->second[3], renameIdx.at( { it->second[4] } ) } ) );
    }

  //std::cout << numInputs << " " << numOutputs << " " << 2 + unordered.size() + 1 << "\n"; //number of Nodes = 2 + unordered.size();

  bddOut << numInputs << " " << numOutputs << " " << 2 + unordered.size() + 1 << "\n"; //number of Nodes = 2 + unordered.size();


  for(unsigned int i = 1; i < order.size() - 1; ++i)
    {
      //std::cout << order[i] << " ";
      bddOut << order[i] << " ";
			      
    }
  
  //std::cout << "\n";
  bddOut << std::endl;

  for(int i = 0; i < numOutputs; ++i)
    {
      //std::cout << (gateBDD[ outputs[i] ]->inv ? "-" : "") << renameIdx.at( { gateBDD[ outputs[i] ]->id } ) << " ";
      bddOut << (gateBDD[ outputs[i] ]->inv ? "-" : "") << renameIdx.at( { gateBDD[ outputs[i] ]->id } ) << " ";	
    }

  //std::cout << "\n";
  bddOut << std::endl;

  
  //_0->printNodeClean();
  //_1->printNodeClean();

  bddOut << _0->id << " " << _0->var << " 0 0" << std::endl;
  bddOut << _1->id << " " << _1->var << " 0 0" << std::endl;


  
  for(auto it = renamed.begin(); it != renamed.end(); ++it)
    {
      //std::cout << it->first << " " << it->second[0] << " " << (it->second[1] ? "-" : "") << it->second[2] << " " << (it->second[3] ? "-" : "") << it->second[4];
      //std::cout << "\n";
      bddOut << it->first << " " << it->second[0] << " " << (it->second[1] ? "-" : "") << it->second[2] << " " << (it->second[3] ? "-" : "") << it->second[4];
      bddOut << std::endl;
    }
  

  //std::cout << "\n\n\n";
  std::ofstream outFile;
  outFile.open(bddFileName);
  outFile << bddOut.rdbuf();
  
}


////////////////////////////////////////////////////////////////////////
// main starts here
////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[])
{
  char cktName[20];
  circuit *ckt;
  std::vector<std::string> argVec(argv + 1, argv + argc);

  auto inputVec = argVec.begin();
  //++inputVec;
		
  if(argc != 3)
    {
      std::cout << "Error! Enter .lev and .vec files as parameters!\n\nUsage: ./a.out <lev_file> <vec_file>\n\n";
      std::cout << "Enter file names without extensions\n";
      
      exit(0);
    }


  strcpy(cktName, argv[1]);	
  //puts(cktName);

  ckt = new circuit(cktName);

  std::vector<int> ord;

  ord.push_back(-1);
  for(int i = 1; i <= ckt->getNumInputs(); i++)
    ord.push_back(i);
  ord.push_back(ckt->getNumInputs() + 1);

  //std::iter_swap(ord.begin() + 1, ord.begin() + 3);


  ckt->setOrder(ord);
  //ckt->setOrder();
  std::cout << "Building for this circuit...." << std::endl;
  ckt->levelizedBuild();
  std::cout << "Build completed!!" << std::endl;
  std::string bddFileName = *inputVec;
  //std::cout << bddFileName;
  std::cout << "Writing to " << bddFileName << ".bdd" << std::endl;
  ckt->print(bddFileName);
  
  //std::cout << vecFileName;
  //std::string vecFileName = *inputVec + ".vec";
  //std::cout << "Simulation of " << vecFileName << ":\n\n";
  ++inputVec; 
  std::string vecFileName = *inputVec;
  std::cout << "Writing simulation to " << vecFileName << ".sim" << std::endl;
  ckt->simulate(vecFileName);
  
  delete ckt;
  
  return 0;
}
