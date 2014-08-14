#ifndef _TREE_H

#define _TREE_H

#include <iostream>
#include <iomanip>
#include <string.h>
#include <math.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <cmath>
#include <set>
#include <list>
#include <sstream>
using namespace std;

class NODE;
class EDGE;

class NODE
{
public:
  int ID, DFSid, DFScolor; //DFScolor: 0-white, 1-grey, 2-black; Depth-first search
  int DFSparent; //ID of DFSparent
  vector <int> GroupA;
  vector <int> GroupB;
  int total;
  int ConnEdgeCount;
  vector <EDGE*> ConnEdges;
  int DistToRoot;
  EDGE* prev;
};

class EDGE
{
public:
  int ID, DFSid, DFSstatus; //DFSstatus: 0-unvisited, 1-discovery edge, 2-backtrack edge, 3-backtrack edge added to the DFSEdgeList
  NODE *OriStart, *OriEnd;
  NODE *CurrStart, *CurrEnd; 
  NODE *plus, *minus; //smaller DFSid tail = plus
  double Weight;
  bool InTree;
};

class NETWORK
{
public:
  int NodeCount, EdgeCount, B, LeftEdgeCount; //LeftEdgeCount: number of edges left after slide.
  vector <NODE*> NodeList;
  vector <EDGE*> EdgeList;
  vector <int> DFSNodeList; //list ID in NodeList, rank_to_node
  vector <int> DFSEdgeList;
  vector <int> node_to_rank, edge_to_rank; // list the rank of the node/edge w.r.t the original node/edge order
  NODE *root;
  vector <int> InTreeEdgeID;
  vector <long double> R0, R1, R2, R3;
  vector <double> R;
  vector <long double> TCount;
  vector < vector<long double> > edge_quantity, edge_quantity_unionMST; // index by edge rank, calculate the associated quantity w.r.t each edge; only calculated for those lefted edges.
  vector <int> node_deg; //index using the original one
  vector <set <int> > candi;
  set <int> leave;
  vector <int> upward_edge; // the unique edge e in T_0 with e_minus(e) = v_i. the first element in the vector will be 0 and has no meaning since no edge point to the root. The element is the rank of the edge.  
  long T0Count;
  vector <double> min_weight;
  set <int> NNB_edge;
public:
  int OpenInFile(ifstream &in, const string &file);
  int InitNodeFromFile(string FilePath, int NodeNum, int nB);
  int InitEdgeFromFile(string FilePath);
  int InitNode(int *NodeInfo, int *NodeNum, int *nB);
  int InitEdge(double *DistInfo);
  int InitMST();
  int InTreeSlide(EDGE *myEdge);
  int OutTreeSlide(EDGE *myEdge);
  int Slide();
  int RemoveEdgeStart(EDGE *myEdge);
  int AddEdgeStart(EDGE *myEdge);
  int RemoveEdgeEnd(EDGE *myEdge);
  int AddEdgeEnd(EDGE *myEdge);
  int RemoveDupEdge(); // to remove duplicated edges (connecting the same two nodes), which may result after the slide transformation
  int GetR0();
  int DFS();
  int runDFS(int k, int DFSNodeCount, int DFSEdgeCount);
  int BacktrackEdgeOrder();
  int GetEdgeQuan();
  int BuildUpwardEdge();
  int GetRank();
  int BuildCandi();
  int Traverse();
  int FindChild();
  int update(int ek, int eg); //remove e_k, add e_g
  int SubChild(int ek, int eg);
  int GetR();
  int power(int base, int b);

  // Given a specific graph structure
  int GetR_GivenEdgeLE(); // LE: left_edges
  int GetR_InitGraph();
  int Get_NNB_edge();
  int GetR_NNB();
  int Get_MinWeight();
  int GetR0_unionMST();
  int GetR_GivenEdgeLE_unionMST();
  int GetEdgeQuan_unionMST();
  ~NETWORK();
};

#endif // _TREE_H
