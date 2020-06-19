////////////////////////////////////////////////////////////////////////
// File: logic simulation
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
#include <unordered_map>

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

public:
    circuit(char *);            // constructor
  void dispGateInfo(int);
  void dfs(int, bool []);
  bool pathExist(int, int);
  void simulate(std::vector<int>, int);

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

    numgates = maxlevels = 0;

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
	    numOutputs++;

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
}

////////////////////////////////////////////////////////////////////////
// other member functions here
////////////////////////////////////////////////////////////////////////

//Function to display details about a gate or port
void circuit::dispGateInfo(int x)
{
  // x is Gate / Port #
  std::cout << "\n";
  std::cout << "Number of FanIns of gate # "<< x << " : " << fanin[x] << "\n";
  std::cout << "Number of FanOuts of gate # "<< x << " : " << fanout[x] << "\n";
  std::cout << "Gate type: " << (int)gtype[x] << "\n";
  std::cout << "FanIn list of gate # " << x << " : ";
  for(int i = 0; i < fanin[x]; ++i)
    std::cout << faninlist[x][i] << " ";
  std::cout << "\n";
  std::cout << "FanOut list of gate # " << x << " : ";
  for(int i = 0; i < fanout[x]; ++i)
    std::cout << fanoutlist[x][i] << " ";
  std::cout << "\n";
  //std::cout << "Count : " << count << "\n";
}

void circuit::dfs(int gate, bool visitedArr[])
{
  visitedArr[gate] = true;
  // Print the gate node being visited
  //std::cout << gate << " ";
  for(int i = 0; i < fanout[gate]; ++i)
    if(!visitedArr[i])
      dfs(fanoutlist[gate][i], visitedArr);
}

// Source = gate, Dest = gate2
bool circuit::pathExist(int gate, int gate2)
{
  if(gate > numgates || gate2 > numgates || gate <= 0 || gate2 <= 0)
    {
      fprintf(stderr, "\nERROR! Gate number not in the .lev file. Enter valid gates. [1 to %d].\n\n", numgates);
      exit(-1);
    }
  
  bool *visitedArr = new bool[numgates + 1];
  // Mark as 'not visited'
  // visitedArr[0] is meaningless. Assuming gate numbers start from number 1 and end with numgates.
  for(int i = 0; i <= numgates; ++i)
    visitedArr[i] = false;

  dfs(gate, visitedArr);
  
  //for(int i = 1; i <= numgates; ++i)
  //  std::cout << "\ngate = " << i << ", " << "Visited? " << visitedArr[i];

  //Return whether destination was visited or not
  return visitedArr[gate2];  
}


void circuit::simulate(std::vector<int> inputVector, int sensitivePort)
{
  std::unordered_map<int, int>logicProp;
  std::unordered_map<int, char>labelProp;
  std::vector<int> outputPorts;
  int eval, op1, op2;
  char lOp1, lOp2, lEval;

  if(inputVector.size() != numInputs)
    {
      std::cout <<"Error!! inputVector.size() = " << inputVector.size() << ", whereas numInputs = " << numInputs << "\n";
      exit(0);
    }

  for(int i = 1; i <= numgates; ++i)
    {
      //std::cout << i << "\n";
      switch(gtype[i])
	{
	  //Input Port
	case 1 : logicProp.insert( { i, inputVector[i - 1] } );
	  if(i == sensitivePort)
	    labelProp.insert( { i, 'H' } );
	  else
	    labelProp.insert( { i, 'L' } );	  
	  break;

	  //Output Port
	case 2 : logicProp.insert( { i, logicProp.at( { faninlist[i][0] } ) } );
	  labelProp.insert( { i, labelProp.at( { faninlist[i][0] } ) } );
	  outputPorts.push_back(i);
	  break;

	  //XOR gate
	case 3 : op1 = logicProp.at( { faninlist[i][0] } );
  	  lOp1 = labelProp.at( { faninlist[i][0] } );

	  for(int j = 1; j < fanin[i]; ++j)
	    {
	      op2 = logicProp.at( { faninlist[i][j] } );
	      eval = op1 ^ op2;
       	      lOp2 = labelProp.at( { faninlist[i][j] } );

	      if(lOp1 == 'L' && lOp2 == 'L')
		lEval = 'L';
	      else if( (lOp1 == 'L' && lOp2 == 'H') || (lOp1 == 'H' && lOp2 == 'L') )
		lEval = 'H';
	      else if(lOp1 == 'H' && lOp2 == 'H')
		lEval = 'H';
	      
	      op1 = eval;
	      lOp1 = lEval;
	      
	    }
	  logicProp.insert( { i, eval } );
	  labelProp.insert( { i, lEval} );
	  break;

 	  //XNOR gate
	case 4 : op1 = logicProp.at( { faninlist[i][0] } );
	  lOp1 = labelProp.at( { faninlist[i][0] } );

	  for(int j = 1; j < fanin[i]; ++j)
	    {
	      op2 = logicProp.at( { faninlist[i][j] } );
	      eval = op1 ^ op2;
	      lOp2 = labelProp.at( { faninlist[i][j] } );

	      if(lOp1 == 'L' && lOp2 == 'L')
		lEval = 'L';
	      else if( (lOp1 == 'L' && lOp2 == 'H') || (lOp1 == 'H' && lOp2 == 'L') )
		lEval = 'H';
	      else if(lOp1 == 'H' && lOp2 == 'H')
		lEval = 'H';
	      
	      op1 = eval;
	      lOp1 = lEval;
	    }
	  eval = eval ? 0 : 1;
	  logicProp.insert( { i, eval } );
	  labelProp.insert( { i, lEval} );
	  break;
	  

	  //AND gate
	case 6 : op1 = logicProp.at( { faninlist[i][0] } );
  	  lOp1 = labelProp.at( { faninlist[i][0] } );

	  for(int j = 1; j < fanin[i]; ++j)
	    {
	      op2 = logicProp.at( { faninlist[i][j] } );
	      eval = op1 & op2;
      	      lOp2 = labelProp.at( { faninlist[i][j] } );


	      if(lOp1 == 'L' && lOp2 == 'L')
		lEval = 'L';
	      else if( (lOp1 == 'L' && op1 == 0) && lOp2 == 'H')
		lEval = 'L';
	      else if( (lOp1 == 'L' && op1 == 1) && lOp2 == 'H')
		lEval = 'H';
	      else if(lOp1 == 'H' && (lOp2 == 'L' && op2 == 0) )
		lEval = 'L';
	      else if(lOp1 == 'H' && (lOp2 == 'L' && op2 == 1) )
		lEval = 'H';
	      else if(lOp1 == 'H' && lOp2 == 'H')
		lEval = 'H';
	      
	      
	      op1 = eval;
	      lOp1 = lEval;
	    }
	  logicProp.insert( { i, eval } );
	  labelProp.insert( { i, lEval} );
	  break;
	  
//for(auto it = logicProp.begin(); it != logicProp.end(); ++it)
  //  std::cout << " " << it->first << " : " << it->second << "\n";	  //NAND gate  
	case 7 : op1 = logicProp.at( { faninlist[i][0] } );
	  lOp1 = labelProp.at( { faninlist[i][0] } );
	  for(int j = 1; j < fanin[i]; ++j)
	    {
	      op2 = logicProp.at( { faninlist[i][j] } );
	      eval = op1 & op2;
  	      lOp2 = labelProp.at( { faninlist[i][j] } );
    	     	      	      
	      if(lOp1 == 'L' && lOp2 == 'L')
		lEval = 'L';
	      else if( (lOp1 == 'L' && op1 == 0) && lOp2 == 'H')
		lEval = 'L';
	      else if( (lOp1 == 'L' && op1 == 1) && lOp2 == 'H')
		lEval = 'H';
	      else if(lOp1 == 'H' && (lOp2 == 'L' && op2 == 0) )
		lEval = 'L';
	      else if(lOp1 == 'H' && (lOp2 == 'L' && op2 == 1) )
		lEval = 'H';
	      else if(lOp1 == 'H' && lOp2 == 'H')
		lEval = 'H';
	      
	      
	      op1 = eval;
	      lOp1 = lEval;
	    }
	  
	  eval = eval ? 0 : 1;
	  logicProp.insert( { i, eval } );
	  labelProp.insert( { i, lEval} );
	  break;
	  
	  //OR gate
	case 8 : op1 = logicProp.at( { faninlist[i][0] } );
  	  lOp1 = labelProp.at( { faninlist[i][0] } );

	  for(int j = 1; j < fanin[i]; ++j)
	    {
	      op2 = logicProp.at( { faninlist[i][j] } );
	      eval = op1 | op2;
	      lOp2 = labelProp.at( { faninlist[i][j] } );


	      if(lOp1 == 'L' && lOp2 == 'L')
		lEval = 'L';
	      else if( (lOp1 == 'L' && op1 == 0) && lOp2 == 'H')
		lEval = 'H';
	      else if( (lOp1 == 'L' && op1 == 1) && lOp2 == 'H')
		lEval = 'L';
	      else if(lOp1 == 'H' && (lOp2 == 'L' && op2 == 0) )
		lEval = 'H';
	      else if(lOp1 == 'H' && (lOp2 == 'L' && op2 == 1) )
		lEval = 'L';
	      else if(lOp1 == 'H' && lOp2 == 'H')
		lEval = 'H';
	      
	      
	      op1 = eval;
	      lOp1 = lEval;
	    }
	  logicProp.insert( { i, eval } );
 	  labelProp.insert( { i, lEval} );
	  break;

	  //NOR gate
	case 9 : op1 = logicProp.at( { faninlist[i][0] } );
	  lOp1 = labelProp.at( { faninlist[i][0] } );

	  for(int j = 1; j < fanin[i]; ++j)
	    {
	      op2 = logicProp.at( { faninlist[i][j] } );
	      eval = op1 | op2;
	      lOp2 = labelProp.at( { faninlist[i][j] } );


	      if(lOp1 == 'L' && lOp2 == 'L')
		lEval = 'L';
	      else if( (lOp1 == 'L' && op1 == 0) && lOp2 == 'H')
		lEval = 'H';
	      else if( (lOp1 == 'L' && op1 == 1) && lOp2 == 'H')
		lEval = 'L';
	      else if(lOp1 == 'H' && (lOp2 == 'L' && op2 == 0) )
		lEval = 'H';
	      else if(lOp1 == 'H' && (lOp2 == 'L' && op2 == 1) )
		lEval = 'L';
	      else if(lOp1 == 'H' && lOp2 == 'H')
		lEval = 'H';
	      
	      
	      op1 = eval;
	      lOp1 = lEval;
	    }
	  eval = eval ? 0 : 1;
	  logicProp.insert( { i, eval } );
	  labelProp.insert( { i, lEval} );
	  break;

	  //NOT gate
	case 10 : op1 = logicProp.at( { faninlist[i][0] } );
	  eval = op1 ? 0 : 1;
	  lOp1 = labelProp.at( { faninlist[i][0] } );
	  lEval = lOp1;
	  logicProp.insert( { i, eval } );
	  labelProp.insert( { i, lEval } );
	  break;

	default:
	  op1 = logicProp.at( { faninlist[i][0] } );
	  eval = op1;
	  logicProp.insert( { i, eval } );
	  
	break;
	}
    }

  //for(auto it = logicProp.begin(); it != logicProp.end(); ++it)
  //  std::cout << " " << it->first << " : " << it->second << "\n";

  //std::cout << "Output: \n\n";
  //std::cout << logicProp.at({12}) << logicProp.at({13}) << "," << labelProp.at({12}) << labelProp.at({13}) << "\n";

  // std::cout << "\nTrace:";

  /*  
  std::cout << "\n\n";
  for(auto it = labelProp.begin(); it != labelProp.end(); ++it)
    {
    //std::cout << " " << it->first << " : " << it->second << "\n";
      std::cout << " " << it->first << " : " << logicProp.at( { it->first } ) << " " << it->second << "\n";
    }
  std::cout << "\n";
  */
  
  //std::cout << logicProp.at({86}) << logicProp.at({136}) << logicProp.at({167}) << logicProp.at({197}) << logicProp.at({201}) << logicProp.at({202}) << logicProp.at({203}) << ",";
  //std::cout << labelProp.at({86}) << labelProp.at({136}) << labelProp.at({167}) << labelProp.at({197}) << labelProp.at({201}) << labelProp.at({202}) << labelProp.at({203}) << "\n";

  //std::cout << logicProp.at({86});

  for(auto i = outputPorts.begin(); i != outputPorts.end(); ++i)
    {
      std::cout << logicProp.at( { *i } );
    }
  /*
  std::cout << ",";
  for(auto i = outputPorts.begin(); i != outputPorts.end(); ++i)
    {
      std::cout << labelProp.at( { *i } );
    }
  */
  std::cout <<"\n";
	
  
  
}

////////////////////////////////////////////////////////////////////////
// main starts here
////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[])
{
    char cktName[20];
    circuit *ckt;
    int vecWidth, poNum, x;
    std::vector<std::string> argVec(argv + 1, argv + argc);

    if (argc != 3)
      {
        fprintf(stderr, "Usage: %s <ckt> <vector>\n", argv[0]);
        exit(-1);
      }

   
    auto inputVec = argVec.begin();
    ++inputVec;
    std::string vecFileName = *inputVec + ".vec";
    //std::cout << vecFileName;
    

    strcpy(cktName, argv[1]);

    // read in the circuit and build data structure
    ckt = new circuit(cktName);
    
        
    // NOW ADD THE REST OF YOUR MAIN() HERE
    
    std::string line;
    std::ifstream vecFile(vecFileName.c_str());
    std::vector<int> vec;


    if(!vecFile.good())
      {
	std::cout << vecFileName << " not found.\n";
	exit(-1);
      }
    /*
    std::cout << "Enter sensitive input port (to be set as 'H'): ";
    std::cin >> x;
    */
    x = 3;

    std::getline(vecFile, line);
    //std::cout << line << "\n";

    while(vecFile.good())
      {
        std::getline(vecFile, line);
        if(line[0] != 'E')
 	 {
 	   //std::cout << line << "\n";
 	   if(vecFile.good())
 	     {
 	       for(int i = 0; i < line.size() - 1; ++i)
 		 {
 		   vec.push_back(line[i] - '0');
 		 }
 	       //std::cout << "\n\nInput Vector = " << vecStr[j] << "\n\n";
 	       ckt->simulate(vec, x);
 	       vec.clear();
 	     }
 	 }
      }
    
   
    

    /*
    std::string line;
    std::ifstream vecFile(vecFileName.c_str());
    
    if(vecFile.good())
      {
	//vecFile.open(vecFileName);
	std::getline(vecFile, line);
	std::cout << line;
	std::getline(vecFile, line);
	std::cout << line;
	vecFile.close();
      }
    else
      {
	std::cout << vecFileName << " not found.\n";
	exit(-1);
      }
    */


    

    /*
    while(std::cin >> x)
       {
        ckt->dispGateInfo(x);
       }
    */

    /*
    std::vector<std::string>  vecStr = {"00001", "00110", "10111", "01011", "11101", "11000", "11110", "10010"};
    std::vector<int> vec;
    
    for(int j = 0; j < vecStr.size(); ++j)
      {
	for(int i = 0; i < vecStr[j].size(); ++i)
	  {
	    vec.push_back(vecStr[j][i] - '0');
	  }
        //std::cout << "\n\nInput Vector = " << vecStr[j] << "\n\n";
	ckt->simulate(vec);
	vec.clear();
	//std::cout << "-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=";
      }
    */
    

    /*
    std::vector<std::string> vecStr = { "000110100001100100110011110101101101",
					"000000000000000000000000000000000000",
					"000000011101110100000110101101101100",
					"101110101101011100110001000110000000",
					"111011110110011010011101101100010001",
					"111110110100111111100110101110111001",
					"011010100011011111000011100010100110",
					"001010010010100010101000101001100111",
					"001010100011110110011100000001011100",
					"101101101011110111110111010100011111",
					"000010111010101111010111000000111000",
					"111010111010001010010001111110110000",
					"000110101011011001110110100001101011",
					"001101111101011010001101100100111011",
					"011001011100000011101001110100110110",
					"010001010000101001000011001010001011",
					"001001111111001100111001100011111100",
					"011011111010101100101001010110011010",
					"000001111010000011001110100001010010",
					"011100011101101101001110010011000111",
					"001001001100010001110100101011001000",
					"011101100110000011000101000111000110",
					"100110000011010110010111010010000011",
					"010101011111001111000111111000100001",
					"101111110001110011010010001001110101",
					"111001101110100101001000100100010010",
					"011010100000111110010100110100100000",
					"001111010100001111101111111000000111",
					"100000100011111001011100001111010100",
					"101000000011101010011000011100111110",
					"101111111110011110100110011110101111",
					"100010001010100010010110000111011000",
					"101011001011000100011110011001000001",
					"011011100001011110110010110011001110",
					"000010000001000010010110101100111110",
					"100101010101110100111111001000001111",
					"110001101111000000110011101010111010",
					"011010001101011011101101000100111011",
					"011111011101011110010110100111010000",
					"001010100010100010110100001111001000",
					"110100000000111100110011100101101011",
					"001001001110010000011111110101111111",
					"111110100000011001100010111011000101",
					"100101000111011110101101111011110010",
					"000000110101000101100011001111010111",
					"111100001010001100101001000000011111",
					"100110111101111111000001010100010010",
					"100111000001111000010000100010011000",
					"111011011101110010111101001101110001",
					"101110010101101110111111111000001001",
					"001010010011010110111110000110010011",
					"000001011110011001110000001111001110", };
    std::vector<int> vec;
     for(int j = 0; j < vecStr.size(); ++j)
      {
	for(int i = 0; i < vecStr[j].size(); ++i)
	  {
	    vec.push_back(vecStr[j][i] - '0');
	  }
	ckt->simulate(vec);
	vec.clear();
      }
     */
    
    

    //std::cout <<"Enter source port and destination port (Ctrl + C to quit):\n";
    //while(std::cin >> gateStart >> gate2)
    //  {
    //	ckt->pathExist(gateStart, gate2) ? std::cout << "\n\n*****\nYES\n*****\n" : std::cout <<"\n\n*****\nNO\n*****\n";
	//std::cout << "\n\n";
    //}
    
    return 0;
}
