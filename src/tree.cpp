#include "tree.h"

extern "C" {

  void RG(double *R, int *Nedges, double *edges, int *edge_set, int *FromFile, int *N, int *nB, int *NodeInfo, double *DistInfo) 
  {
    // edge_set[0]: 0 - aMST; 1 - G-uMST; 2 - initial graph; 3 - G-uNNB; 4 - uMST
    // N: number of nodes
    // FromFile: 0 - not from file; 1 - from file.
    // Nedges, edges: the number of edges and the edge pairs (only specified when edge_set is 0 or 3)
    string File1 = "node.txt";
    string File2 = "dist.txt";
    int NodeNum = 9;
    int B = 2;
    NodeNum = N[0];
    B = nB[0];

    NETWORK graph;
    
    // ofstream progstream;
    // progstream.open("MST.txt");
    // progstream << "Begin all \n";
    if (FromFile[0]==1){
      graph.InitNodeFromFile(File1, NodeNum, B);
      graph.InitEdgeFromFile(File2);
    }else{
      graph.InitNode(NodeInfo, N, nB);
      graph.InitEdge(DistInfo);
    }
    if (edge_set[0] == 4){
      graph.GetR0_unionMST();
    }else{
      graph.GetR0();
    }
    if (edge_set[0] == 3){
      graph.GetR_NNB();
      int i = 0;
      for (set<int>::iterator ei=graph.NNB_edge.begin(); ei!=graph.NNB_edge.end(); ei++){
        edges[2*i] = graph.EdgeList[*ei]->OriStart->ID;
        edges[2*i+1] = graph.EdgeList[*ei]->OriEnd->ID;
        i++;
      }
      Nedges[0] = i;
    }else{
      graph.InitMST();
      graph.Slide();
      graph.DFS();
      graph.BacktrackEdgeOrder();  
    
      if (edge_set[0] == 0){ 
        graph.Traverse();
        graph.GetR();
      }else if (edge_set[0]==1){
        graph.GetEdgeQuan();
        graph.GetR_GivenEdgeLE();
        for (int i=0; i<graph.LeftEdgeCount; i++){
          int e = graph.DFSEdgeList[i];
          edges[2*i] = graph.EdgeList[e]->OriStart->ID;
          edges[2*i+1] = graph.EdgeList[e]->OriEnd->ID;         
        }
        Nedges[0] = graph.LeftEdgeCount;
      }else if (edge_set[0]==4){       
        graph.GetEdgeQuan_unionMST();
        graph.GetR_GivenEdgeLE_unionMST();
      }else if (edge_set[0] == 2){
        graph.GetR_InitGraph();
      }
    }
    // ofstream progstream;
    // progstream.open("DFS.txt");
    // for (int i=0; i<graph.NodeCount; i++){
    //   progstream << graph.DFSEdgeList[i] << ", ";
    // }
    // progstream.close();
    
    for (int b=0; b<B; b++){
      R[b] = graph.R[b];
    }
    // progstream << "NodeCount "<<graph.NodeCount <<"\n";
    // progstream << "EdgeCount" <<graph.EdgeCount <<"\n";
    // progstream.close();
  }


  int NETWORK::OpenInFile(ifstream &in, const string &file)
  {
    in.open(file.c_str(),ios::in);
    if(!in) 
    {
      // comment this for submitting to CRAN
      // cout<<"\n\tCannot open file "<<file<<" to read."<<endl;
      return 0;
    }
    return 1;
  }

  int NETWORK::InitNode(int *NodeInfo, int *NodeNum, int *nB)
  {
    NodeCount = NodeNum[0];
    B = nB[0];
    for (int i=0; i<NodeCount; i++){
      NODE *myNode;
      myNode = new NODE;
      myNode->ID = i;
      myNode->DFScolor = 0;
      vector <int> GroupA(B), GroupB(B);
      for (int b=0; b<B; b++){
        int idA = i + b*2*NodeCount;
        int idB = i + (b*2+1)*NodeCount;
        GroupA[b] = NodeInfo[idA];
        GroupB[b] = NodeInfo[idB];
      }
      myNode->GroupA = GroupA;
      myNode->GroupB = GroupB;
      myNode->total = NodeInfo[i] + NodeInfo[i+NodeCount];
      myNode->ConnEdgeCount = 0;
      myNode->ConnEdges.clear();
      myNode->prev = NULL;
      NodeList.push_back(myNode);
    }
    root = NodeList[0];
    NodeList[0]->DistToRoot=0;
    //  R0 = R = 0.0;
    return 1;
  }

  int NETWORK::InitNodeFromFile(string FilePath, int NodeNum, int nB)
  {
    B = nB;
    ifstream GroupABFile;
    if(!OpenInFile(GroupABFile,FilePath)){
      return 0;
    }
    NodeCount = NodeNum;
	
    for (int i=0;i<NodeNum;i++)
    {
      int ga, gb;
      NODE *myNode;
      myNode = new NODE;
      myNode->ID = i;
      myNode->DFScolor = 0;
      vector <int> GroupA(B), GroupB(B);
      for (int b=0; b<B; b++){
        GroupABFile >> ga;
        GroupABFile >> gb;
        GroupA[b] = ga;
        GroupB[b] = gb;
      }     
      myNode->GroupA = GroupA;
      myNode->GroupB = GroupB;
      myNode->total = ga + gb;
      myNode->ConnEdgeCount = 0;//to be changed in InitEdge(string FilePath)
      myNode->ConnEdges.clear();//to be changed in InitEdge(string FilePath)
      myNode->prev = NULL; // to be changed in InitMST()
      NodeList.push_back(myNode);
    }
    GroupABFile.close();
    root = NodeList[0];
    NodeList[0]->DistToRoot = 0;
    //   R0 = R = 0.0;
    return 1;
  }

  int NETWORK::InitEdge(double *DistInfo) //DistInfo contains the distance information, vectorized by rows. "-1" denote no distance information available for that pair of nodes.
  {
    int count = 0;
    for (int i=0; i<NodeCount; i++){
      for (int j=0; j<NodeCount; j++){
        int tmp = i*NodeCount + j;
        if (j>i && DistInfo[tmp]!= -1){
          EDGE *myEdge;
          myEdge = new EDGE;
          myEdge->ID = count;
          myEdge->DFSstatus = 0;
          myEdge->OriStart  = (NodeList[i]);
          myEdge->OriEnd    = (NodeList[j]);
          myEdge->CurrStart = (NodeList[i]);
          myEdge->CurrEnd   = (NodeList[j]);
          myEdge->Weight    = DistInfo[tmp];
          myEdge->InTree    = false;
          EdgeList.push_back(myEdge);
          NodeList[i]->ConnEdgeCount++;
          NodeList[j]->ConnEdgeCount++;
          NodeList[i]->ConnEdges.push_back((EdgeList[count]));
          NodeList[j]->ConnEdges.push_back((EdgeList[count]));
          count++;
        }
      }
    }
    EdgeCount = count;
    return 1;
  }
              
  
  int NETWORK::InitEdgeFromFile(string FilePath)
  {
    ifstream MatrixFile;
    if(!OpenInFile(MatrixFile,FilePath)){
      return 0;
    }
    string temp;
    int count = 0;
    for (int i=0; i<NodeCount; i++){
      for (int j=0; j<NodeCount; j++){
        MatrixFile>>temp;
        if (j>i && temp!="-1" && temp!="NA"){ 
          EDGE *myEdge;
          myEdge = new EDGE[1];
          myEdge->ID	       	=  count;
          myEdge->DFSstatus = 0;
          myEdge->OriStart	=  (NodeList[i]);
          myEdge->OriEnd	=  (NodeList[j]);
          myEdge->CurrStart	=  (NodeList[i]);
          myEdge->CurrEnd	=  (NodeList[j]);
          istringstream stm;
          stm.str(temp);
          stm >> myEdge->Weight;
          myEdge->InTree       	= false;
          EdgeList.push_back(myEdge);
          NodeList[i]->ConnEdgeCount++;
          NodeList[j]->ConnEdgeCount++;
          NodeList[i]->ConnEdges.push_back( (EdgeList[count]));
          NodeList[j]->ConnEdges.push_back( (EdgeList[count]));
          count++;
        }
      }
    }
    EdgeCount = count;
    MatrixFile.close();
    return 0;
  }

  int NETWORK::InitMST() //Prim's algorithm
  {
    vector <int> nodeIn;
    nodeIn.clear();
    nodeIn.resize(NodeCount);
    nodeIn[0] = 1;
    InTreeEdgeID.clear();
    InTreeEdgeID.resize(NodeCount-1);
    for (int i=1; i<NodeCount; i++){
      EDGE *inEdge;
      bool flag = false;
      for (int j=0; j<NodeCount; j++){
        if (nodeIn[j]){
          for (int k=0; k<NodeList[j]->ConnEdgeCount; k++){
            EDGE *myEdge;
            myEdge = NodeList[j]->ConnEdges[k];
            if (!myEdge->InTree && !(nodeIn[myEdge->OriEnd->ID] && nodeIn[myEdge->OriStart->ID])){ // nodeIn[myEdge->OriStart->ID] is useful in case when a later added node add into nodeIn first.
              if (!flag){
                inEdge = myEdge;
                flag = true;
              }else{
                if (inEdge->Weight > myEdge->Weight){
                  inEdge = myEdge;
                }
              }
            }
          }
        }
      }
      inEdge->InTree = true;
      if (nodeIn[(inEdge->OriStart->ID)]){
        nodeIn[(inEdge->OriEnd->ID)] = 1;	
        inEdge->OriEnd->DistToRoot = i;
      }else{
        nodeIn[(inEdge->OriStart->ID)] = 1;
        inEdge->OriStart->DistToRoot = i;
        inEdge->CurrStart =  (NodeList[(inEdge->OriEnd->ID) ]);
        inEdge->CurrEnd   =  (NodeList[(inEdge->OriStart->ID) ]);
      }
      NodeList[(inEdge->CurrEnd->ID)]->prev = inEdge;
      InTreeEdgeID[i-1] = inEdge->ID;
    }
    return 1;
  }

  int NETWORK::RemoveEdgeStart(EDGE *myEdge)
  {
    vector<EDGE*>::iterator i;
    for (i=myEdge->CurrStart->ConnEdges.begin(); i!=myEdge->CurrStart->ConnEdges.end(); i++)
    {
      EDGE *a = *i;
      if (myEdge->ID == a->ID)
      {
        myEdge->CurrStart->ConnEdges.erase(i);
        myEdge->CurrStart->ConnEdgeCount--;
        break;
      }
    }
    return 1;
  }

  int NETWORK::AddEdgeStart(EDGE *myEdge)
  {
    myEdge->CurrStart->ConnEdges.push_back(myEdge);
    myEdge->CurrStart->ConnEdgeCount++;
    return 1;
  }

  int NETWORK::RemoveEdgeEnd(EDGE *myEdge)
  {
    vector<EDGE*>::iterator i;
    for (i=myEdge->CurrEnd->ConnEdges.begin(); i!=myEdge->CurrEnd->ConnEdges.end(); i++)
    {
      EDGE *a = *i;
      if (myEdge->ID == a->ID)
      {
        myEdge->CurrEnd->ConnEdges.erase(i);
        myEdge->CurrEnd->ConnEdgeCount--;
        break;
      }
    }
    return 1;
  }

  int NETWORK::AddEdgeEnd(EDGE *myEdge)
  {
    myEdge->CurrEnd->ConnEdges.push_back(myEdge);
    myEdge->CurrEnd->ConnEdgeCount++;
    return 1;
  }


  int NETWORK::InTreeSlide(EDGE *myEdge)
  {
    bool flag = true;
    while (myEdge->CurrStart->ID != 0)
    {
      if (myEdge->Weight > myEdge->CurrStart->prev->Weight)
      {
        if (flag)
        {
          RemoveEdgeStart(myEdge);
          flag = false;
        }
        myEdge->CurrStart = myEdge->CurrStart->prev->CurrStart;			
      }else{
        if (!flag)
        {
          AddEdgeStart(myEdge);
          flag = true;
        }
        return 1;
      }
    }
    if (!flag)
    {
      AddEdgeStart(myEdge);
    }
    return 1;
  }

  int NETWORK::OutTreeSlide(EDGE *myEdge)
  {
    bool flag = true;
    while (myEdge->CurrEnd->ID != 0)
    {
      if (myEdge->Weight > myEdge->CurrEnd->prev->Weight)
      {
        if (flag)
        {
          RemoveEdgeEnd(myEdge);
          flag = false;
        }
        myEdge->CurrEnd = myEdge->CurrEnd->prev->CurrStart;
        if (myEdge->CurrEnd->ID == myEdge->CurrStart->ID)
        {
          RemoveEdgeStart(myEdge);
          return 0;
        }
      }else{
        if (!flag)
        {
          AddEdgeEnd(myEdge);
          flag = true;
        }
        break;
      }
    }
    if (!flag)
    {
      AddEdgeEnd(myEdge);
      flag = true;
    }
    while (myEdge->CurrStart->ID != 0)
    {
      if (myEdge->Weight > myEdge->CurrStart->prev->Weight)
      {
        if (flag)
        {
          RemoveEdgeStart(myEdge);
          flag = false;
        }
        myEdge->CurrStart = myEdge->CurrStart->prev->CurrStart;
        if (myEdge->CurrStart->ID == myEdge->CurrEnd->ID)
        {
          RemoveEdgeEnd(myEdge);
          return 0;
        }
      }else{
        if (!flag)
        {
          AddEdgeStart(myEdge);
          flag = true;
        }
        break;
      }
    }
    if (!flag)
    {
      AddEdgeStart(myEdge);
      flag = true;
    }
    return 1;
  }

  int NETWORK::Slide()
  {
    for (int i=0; i<EdgeCount; i++)
    {
      if (EdgeList[i]->InTree)
      {
        InTreeSlide(EdgeList[i]);
      } 
      else
      {
        OutTreeSlide(EdgeList[i]);		
      }
    }
    //   RemoveDupEdge();
    return 1;
  }

  int NETWORK::RemoveDupEdge()
  {
    for (int i=0; i<NodeCount; i++){
      if (NodeList[i]->ConnEdgeCount > 1){
        vector <int> a(NodeCount);
        for (int j=0; j<NodeList[i]->ConnEdgeCount; j++){
          EDGE *myEdge = NodeList[i]->ConnEdges[j];
          int id1 = myEdge->CurrStart->ID;
          int id2 = myEdge->CurrEnd->ID;
          int id = id1;
          if (id1 == i)  id = id2;
          if (a[id] == 1){
            RemoveEdgeStart(myEdge);
            RemoveEdgeEnd(myEdge);
            j--;
          }else{
            a[id] = 1;
          }
        }
      }
    }
    return 1;
  }

  int NETWORK::GetR0()
  {
    R0.clear();
    R0.resize(B);
    for (int b=0; b<B; b++){
      for (int i=0; i<NodeCount; i++)
      {
        R0[b] = R0[b] + 2.0*double(NodeList[i]->GroupA[b])*double(NodeList[i]->GroupB[b])/double(NodeList[i]->total);
      }
    }
    return 1;
  }

  int NETWORK::DFS()
  {
    DFSNodeList.clear();
    DFSEdgeList.clear();
    DFSNodeList.push_back(0);
    NodeList[0]->DFScolor = 1;
    NodeList[0]->DFSid = 0;
    int DFSNodeCount = 1;
    int DFSEdgeCount = 0;
    runDFS(0, DFSNodeCount, DFSEdgeCount);
    return 1;
  }

  int NETWORK::runDFS(int k, int DFSNodeCount, int DFSEdgeCount)
  {
    NodeList[k]->DFScolor = 1;
    if (NodeList[k]->DFScolor == 2){
      runDFS(NodeList[k]->DFSparent, DFSNodeCount, DFSEdgeCount);
      return 0;
    }
    for (int i=0; i<NodeList[k]->ConnEdgeCount; i++){
      if (NodeList[k]->ConnEdges[i]->DFSstatus == 0){
        if ((NodeList[k]->ConnEdges[i]->CurrStart->ID==k && NodeList[k]->ConnEdges[i]->CurrEnd->DFScolor==0) || (NodeList[k]->ConnEdges[i]->CurrEnd->ID==k && NodeList[k]->ConnEdges[i]->CurrStart->DFScolor==0)){
          NodeList[k]->ConnEdges[i]->DFSstatus = 1;
          NodeList[k]->ConnEdges[i]->DFSid = DFSEdgeCount;
          DFSEdgeCount++;
          DFSEdgeList.push_back(NodeList[k]->ConnEdges[i]->ID);
          NodeList[k]->ConnEdges[i]-> plus = NodeList[k];        
          int newID;
          if (NodeList[k]->ConnEdges[i]->CurrStart->ID==k){
            newID = NodeList[k]->ConnEdges[i]->CurrEnd->ID;
          }else{
            newID = NodeList[k]->ConnEdges[i]->CurrStart->ID;
          }
          NodeList[k]->ConnEdges[i]->minus = NodeList[newID];
          NodeList[k]->ConnEdges[i]->plus->DFSid = DFSNodeCount;
          NodeList[k]->ConnEdges[i]->minus->DFSid = DFSNodeCount+1;
          DFSNodeList.push_back(newID);
          NodeList[newID]->DFScolor = 1;
          DFSNodeCount++;
          NodeList[newID]->DFSparent = k;
          runDFS(newID, DFSNodeCount, DFSEdgeCount);
          return 0;
        }else{
          NodeList[k]->ConnEdges[i]->DFSstatus = 2;
          int a = NodeList[k]->ConnEdges[i]->CurrStart->DFSid;
          int b = NodeList[k]->ConnEdges[i]->CurrEnd->DFSid;
          if (a<b){
            NodeList[k]->ConnEdges[i]->plus = NodeList[NodeList[k]->ConnEdges[i]->CurrStart->ID];
            NodeList[k]->ConnEdges[i]->minus = NodeList[NodeList[k]->ConnEdges[i]->CurrEnd->ID];
            NodeList[k]->ConnEdges[i]->plus->DFSid = a;
            NodeList[k]->ConnEdges[i]->minus->DFSid = b;
          }else{
            NodeList[k]->ConnEdges[i]->plus = NodeList[NodeList[k]->ConnEdges[i]->CurrEnd->ID];
            NodeList[k]->ConnEdges[i]->minus = NodeList[NodeList[k]->ConnEdges[i]->CurrStart->ID];  
            NodeList[k]->ConnEdges[i]->plus->DFSid = b;
            NodeList[k]->ConnEdges[i]->minus->DFSid = a;
          }
        }        
      }
    }
    NodeList[k]->DFScolor = 2;
    if (k != 0){
      runDFS(NodeList[k]->DFSparent, DFSNodeCount, DFSEdgeCount);
      return 0;
    }
    /*  }else{
        LeftEdgeCount = DFSEdgeCount;
        }*/
    return 1;
  }

  int NETWORK::BacktrackEdgeOrder()
  {
    int myCount = NodeCount-1;
    for (int i=0; i<NodeCount; i++){
      int myID = DFSNodeList[i];
      for (int j=0; j<NodeList[myID]->ConnEdgeCount; j++){
        if (NodeList[myID]->ConnEdges[j]->DFSstatus == 2){
          DFSEdgeList.push_back(NodeList[myID]->ConnEdges[j]->ID);
          NodeList[myID]->ConnEdges[j]->DFSid = myCount;
          NodeList[myID]->ConnEdges[j]->DFSstatus = 3;
          myCount++;
        }
      }
    }
    LeftEdgeCount = myCount;
    return 1; 
  }    

  int NETWORK::BuildUpwardEdge() //the first element in the vector will be 0 and has no meaning since no edge point to the root. The element is the rank of the edge.
  {
    upward_edge.resize(NodeCount);
    for (int i=0; i<NodeCount-1; i++){
      int e = DFSEdgeList[i];
      upward_edge[EdgeList[e]->minus->ID] = i;
    }
    return 1;
  }

  int NETWORK::GetRank()
  {
    node_to_rank.resize(NodeCount);
    for (int i=0; i<NodeCount; i++){
      int j = DFSNodeList[i];
      node_to_rank[j] = i;
    }
    edge_to_rank.resize(EdgeCount); // some edges are moved from the slide and thus do not have rank.
    for (int i=0; i<LeftEdgeCount; i++){
      int j = DFSEdgeList[i];
      edge_to_rank[j] = i;
    }
    return 1;
  }
  

  int NETWORK::BuildCandi() //and leave, the index in candi is rank of the edges, and the number in the set of each element in candi is also rank of the edges, and the number in leave is also rank of edges
  {
    candi.resize(NodeCount-1);
    for (int i=0; i<NodeCount-1; i++){
      int e1 = DFSEdgeList[i];
      for (int j=NodeCount-1; j<LeftEdgeCount; j++){
        int e2 = DFSEdgeList[j];
        if (EdgeList[e1]->minus->ID == EdgeList[e2]->minus->ID && node_to_rank[EdgeList[e1]->plus->ID] >= node_to_rank[EdgeList[e2]->plus->ID]){
          candi[i].insert(j);
        }
      }
      if (!candi[i].empty()){
        leave.insert(i);
      }
    }
    return 1;
  }


  int NETWORK::Traverse()
  {
    GetEdgeQuan();
    BuildUpwardEdge();
    GetRank();
    BuildCandi();

    node_deg.resize(NodeCount);
    R1.resize(B);
    TCount.resize(B);
    for (int i=0; i<NodeCount-1; i++){
      // cur_tree_edges.insert(i);
      for (int b=0; b<B; b++){
        R1[b] = R1[b] + edge_quantity[i][b];
      }
      // node_deg[EdgeList[DFSEdgeList[i]]->plus->ID]++;
      // node_deg[EdgeList[DFSEdgeList[i]]->minus->ID]++;
      node_deg[EdgeList[DFSEdgeList[i]]->OriStart->ID]++;
      node_deg[EdgeList[DFSEdgeList[i]]->OriEnd->ID]++;
    }
    R2.resize(B);
    for (int b=0; b<B; b++){
      R2[b] = 1;
      for (int i=0; i<NodeCount; i++){
        R2[b] *= power((NodeList[i]->GroupA[b] + NodeList[i]->GroupB[b]), node_deg[i]);
      }
    }
    
    // Rvec.clear();

    R3.resize(B);
    for (int b=0; b<B; b++){
      R3[b] = R1[b]*R2[b];
      TCount[b] += R2[b];
    }
    // Rvec.push_back(tmp);
    
    T0Count = 1;
  
    FindChild();
    return 1;
  }

  int NETWORK::FindChild()
  {
    if (leave.empty()){
      return 1;
    }

    list <int> Q;

    // e_k = the last entry of leave, which is also the rank of the edge
    int ek = *leave.rbegin();

    // delete e_k from leave
    leave.erase(ek);

    while ( !candi[ek].empty() ){
      // e_g = the last entry of candi(e_k)
      int eg = *candi[ek].rbegin();
      // delete e_g from candi(e_k)
      candi[ek].erase(eg);
      // add e_g to the beginning of Q
      Q.push_front(eg);

      T0Count++;
 
      update(ek, eg);
      for (int b=0; b<B; b++){
        R3[b] += R1[b]*R2[b];
        TCount[b] += R2[b];
      }
        
      SubChild(ek, eg);

      update(eg, ek);

    }

    // move all entries of Q to candi(e_k)
    candi[ek].insert( Q.begin(), Q.end() );
    Q.clear();

    // find children of T^p containing e_k
    SubChild(ek, ek);

    // add e_k to the end of leave
    leave.insert(ek);

    return 1;
  }

  int NETWORK::update(int ek, int eg)
  {
    // int n1 = EdgeList[DFSEdgeList[ek]]->plus->ID;
    // int n2 = EdgeList[DFSEdgeList[ek]]->minus->ID;
    // int n3 = EdgeList[DFSEdgeList[eg]]->plus->ID;
    // int n4 = EdgeList[DFSEdgeList[eg]]->minus->ID;
    int n1 = EdgeList[DFSEdgeList[ek]]->OriStart->ID;
    int n2 = EdgeList[DFSEdgeList[ek]]->OriEnd->ID;
    int n3 = EdgeList[DFSEdgeList[eg]]->OriStart->ID;
    int n4 = EdgeList[DFSEdgeList[eg]]->OriEnd->ID;
    node_deg[n1]--;
    node_deg[n2]--;
    node_deg[n3]++;
    node_deg[n4]++;

    for (int b=0; b<B; b++){
      R1[b] = R1[b] - edge_quantity[ek][b] + edge_quantity[eg][b];
      R2[b] = R2[b]*(NodeList[n3]->GroupA[b]+NodeList[n3]->GroupB[b])*(NodeList[n4]->GroupA[b]+NodeList[n4]->GroupB[b])/(NodeList[n1]->GroupA[b]+NodeList[n1]->GroupB[b])/(NodeList[n2]->GroupA[b]+NodeList[n2]->GroupB[b]);
    }
    return 1;
  }
  

  int NETWORK::SubChild(int ek, int eg)
  {
    if (candi[ek].empty() || node_to_rank[EdgeList[DFSEdgeList[eg]]->plus->ID] <= node_to_rank[EdgeList[DFSEdgeList[*candi[ek].begin()]]->plus->ID]){
      FindChild();
      return 0;
    }

    // f = the edge in T^0 with f- = eg+
    int ef = upward_edge[EdgeList[DFSEdgeList[eg]]->plus->ID];

    if ( !candi[ef].empty() ){
      set <int> S;
      for (set<int>::iterator ei = candi[ek].begin(); ei != candi[ek].end(); ei++)
      {
        if (node_to_rank[EdgeList[DFSEdgeList[*ei]]->plus->ID] < node_to_rank[EdgeList[DFSEdgeList[eg]]->plus->ID] ){
          S.insert(*ei);
        }
      }
      // merge S into candi(e_f)
      candi[ef].insert(S.begin(), S.end());

      FindChild();

      // delete all entries of S from candi(e_f)
      for (set<int>::iterator ei = S.begin(); ei != S.end(); ei++){
        candi[ef].erase(*ei);
      }
      S.clear();
    }else{
      for (set<int>::iterator ei = candi[ek].begin(); ei != candi[ek].end(); ei++){
        if (node_to_rank[EdgeList[DFSEdgeList[*ei]]->plus->ID] < node_to_rank[EdgeList[DFSEdgeList[eg]]->plus->ID] ){
          candi[ef].insert(*ei);
        }
      }
      leave.insert(ef);
      FindChild();
      leave.erase(ef);
      candi[ef].clear();
    }
    return 1;
  }

  int NETWORK::GetR()
  {
    R.resize(B);
    for (int b=0; b<B; b++){
      R[b] = R0[b] + R3[b]/TCount[b];
    }
    return 1;
  }
      
  int NETWORK::power(int base, int b)
  {
    if (b == 1)
    {
      return base;
    } 
    else
    {
      return (base*power(base, b-1));
    }
  }

  int NETWORK::GetEdgeQuan()
  {
    edge_quantity.resize(LeftEdgeCount);
    for (int i=0; i<LeftEdgeCount; i++){
      edge_quantity[i].resize(B);
      for (int b=0; b<B; b++){
        int e = DFSEdgeList[i];
        // int n1A = NodeList[EdgeList[e]->plus->ID]->GroupA[b];
        // int n1B = NodeList[EdgeList[e]->plus->ID]->GroupB[b];
        // int n2A = NodeList[EdgeList[e]->minus->ID]->GroupA[b];
        // int n2B = NodeList[EdgeList[e]->minus->ID]->GroupB[b];      
        int n1A = NodeList[EdgeList[e]->OriStart->ID]->GroupA[b];
        int n1B = NodeList[EdgeList[e]->OriStart->ID]->GroupB[b];
        int n2A = NodeList[EdgeList[e]->OriEnd->ID]->GroupA[b];
        int n2B = NodeList[EdgeList[e]->OriEnd->ID]->GroupB[b];      
        edge_quantity[i][b] = 1.0*(n1A*n2B + n2A*n1B)/((n1A+n1B)*(n2A+n2B));
      }
    }
    return 1;
  }

  int NETWORK::GetR_GivenEdgeLE()
  {
    R.resize(B);
    for (int b=0; b<B; b++){
      R[b] = R0[b];
      for (int i=0; i<LeftEdgeCount; i++){
        R[b] += edge_quantity[i][b];
      }
    }
    return 1;
  }

  int NETWORK::GetR_InitGraph()
  {
    R.resize(B);
    for (int b=0; b<B; b++){
      R[b] = R0[b];
      for (int i=0; i<EdgeCount; i++){
        int n1A = EdgeList[i]->OriStart->GroupA[b];
        int n1B = EdgeList[i]->OriStart->GroupB[b];
        int n2A = EdgeList[i]->OriEnd->GroupA[b];
        int n2B = EdgeList[i]->OriEnd->GroupB[b];
        R[b] += 1.0*(n1A*n2B + n1B*n2A)/((n1A+n1B)*(n2A+n2B));
      }
    }
    return 1;
  }
      
  int NETWORK::GetR_NNB()
  {
    Get_MinWeight();
    NNB_edge.clear();
    for (int i=0; i<NodeCount; i++){
      for (int j=0; j<NodeList[i]->ConnEdgeCount; j++){
        if (NodeList[i]->ConnEdges[j]->Weight == min_weight[i]){
          NNB_edge.insert(NodeList[i]->ConnEdges[j]->ID);
        }
      }       
    }
    R.resize(B);
    for (int b=0; b<B; b++){
      R[b] = R0[b];
      for (set<int>::iterator ei=NNB_edge.begin(); ei!=NNB_edge.end(); ei++){
        int n1A = EdgeList[*ei]->OriStart->GroupA[b];
        int n1B = EdgeList[*ei]->OriStart->GroupB[b];
        int n2A = EdgeList[*ei]->OriEnd->GroupA[b];
        int n2B = EdgeList[*ei]->OriEnd->GroupB[b];
        R[b] += 1.0*(n1A*n2B + n1B*n2A)/((n1A+n1B)*(n2A+n2B));
      }
    }
    return 1;
  }

  int NETWORK::Get_MinWeight()
  {
    min_weight.resize(NodeCount);
    for (int i=0; i<NodeCount; i++){
      min_weight[i] = NodeList[i]->ConnEdges[0]->Weight;
      if (NodeList[i]->ConnEdgeCount > 1){
        for (int j=1; j<NodeList[i]->ConnEdgeCount; j++){
          if (min_weight[i] > NodeList[i]->ConnEdges[j]->Weight){
            min_weight[i] = NodeList[i]->ConnEdges[j]->Weight;
          }
        }
      }
    }
    return 1;
  }

  int NETWORK::GetR0_unionMST()
  {
    R0.clear();
    R0.resize(B);
    for (int b=0; b<B; b++){
      for (int i=0; i<NodeCount; i++)
      {
        R0[b] = R0[b] + 1.0*double(NodeList[i]->GroupA[b])*double(NodeList[i]->GroupB[b]);
      }
    }
    return 1; 
  }

  int NETWORK::GetEdgeQuan_unionMST()
  {
    edge_quantity_unionMST.resize(LeftEdgeCount);
    for (int i=0; i<LeftEdgeCount; i++){
      edge_quantity_unionMST[i].resize(B);
      for (int b=0; b<B; b++){
        int e = DFSEdgeList[i];
        int n1A = NodeList[EdgeList[e]->OriStart->ID]->GroupA[b];
        int n1B = NodeList[EdgeList[e]->OriStart->ID]->GroupB[b];
        int n2A = NodeList[EdgeList[e]->OriEnd->ID]->GroupA[b];
        int n2B = NodeList[EdgeList[e]->OriEnd->ID]->GroupB[b];      
        edge_quantity_unionMST[i][b] = 1.0*(n1A*n2B + n2A*n1B);
      }
    }
    return 1;
  }

  int NETWORK::GetR_GivenEdgeLE_unionMST()
  {
    R.resize(B);
    for (int b=0; b<B; b++){
      R[b] = R0[b];
      for (int i=0; i<LeftEdgeCount; i++){
        R[b] += edge_quantity_unionMST[i][b];
      }
    }
    return 1;
  }

  NETWORK::~NETWORK()
  {
    for (int i=0; i<NodeCount; i++) delete NodeList[i];
    for (int i=0; i<EdgeCount; i++) delete EdgeList[i];
  }
}
