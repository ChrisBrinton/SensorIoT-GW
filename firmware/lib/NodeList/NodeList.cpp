//*******************************************************************
//  NodeList.cpp
//  NodeList_Project
//
//  Created by Karlina Beringer on June 12, 2014.
//  This source file contains the NodeList class definitions.
//  Modified by Chris Brinton 2017-1019 to include node meta data.
//*******************************************************************

#include "NodeList.h"


// Default Constructor creates the head node.
NodeList::NodeList()
{
    head = new node;
    head->nodeID = 0;
    head->next = 0;
    listLength = 0;
    nodeptr = head;
}

// Setter adds a node to the list at a given position.
// Takes a node and list position as parameters.
// Position must be between 1 and the number of data nodes.
// Returns true if the operation is successful.
bool NodeList::insertNode(int newNode)
{
    //DEBUG_MSG("NodeList::insertNode: %i\n", newNode);
    if(listLength == 0){
      head->nodeID = newNode;
      listLength++;
      return true;
    }
    int count = 0;
    node* p = head;
    node* q = head;
    while (q)
    {
        if (q->next == 0)
        {
            p = new node;
            p->nodeID = newNode;
            p->next = 0;
            q->next = p;
            listLength++;
            return true;
        }
        p = q;
        q = p->next;
    }
    return true;
}

bool NodeList::nodeExists(int nodeID)
{
  //DEBUG_MSG("NodeList::nodeExists: %i\n", nodeID);
  node* p = head;
  node* q = head;
  while(q)
  {
    if(q->nodeID == nodeID){
      return true;
    }
    q = p->next;
    p = q;
  }
  return false;
}

bool NodeList::setUpdateTime(int nodeID, time_t tUpdate)
{
  node* p = head;
  node* q = head;
  while(q)
  {
    if(q->nodeID == nodeID){
      q->tAge = tUpdate;
      return true;
    }
    q = p->next;
    p = q;
  }
  return false;
}

int NodeList::getNodeUpdateAge(int nodeID)
{
  node* p = head;
  node* q = head;
  int iNowMin, iUpdateMin, iUpdateAge;
  while(q)
  {
    if(q->nodeID == nodeID){
      time_t tNow = time(NULL);
      struct tm *tmNow = localtime(&tNow);
      iNowMin = tmNow->tm_hour*60+tmNow->tm_min;
      struct tm *tmThen = localtime(&q->tAge);
      iUpdateMin = tmThen->tm_hour*60+tmThen->tm_min;
      //DEBUG_MSG("NodeList::getNodeUpdateAge: iNowMin:%i iUpdateMin:%i\n", iNowMin, iUpdateMin);
      iUpdateAge = iNowMin - iUpdateMin;
      return iUpdateAge;
    }
    q = p->next;
    p = q;
  }
  return 0;
}

int NodeList::currentNode()
{
  if(nodeptr==0){
    return 0;
  }
  return nodeptr->nodeID;
}

int NodeList::nextNode()
{
  if(nodeptr==0){
    return 0;
  }
  node* tmpNode = nodeptr;
  if(nodeptr->next == 0){
    nodeptr = head;
  }
  else{
    nodeptr = tmpNode->next;
  }
  return tmpNode->nodeID;
}

bool NodeList::deleteNode(int iNodeID)
{
  node* p = head;
  node* q = head;
  node* last = head;
  while(q)
  {
    if(q->nodeID == iNodeID){
      //check if there is a node after this one. If so, relink the list without this node and delete the node
      if(q->next){
        last->next = q->next;
        if(nodeptr == q)
          nodeptr = last->next; //set the current location to the node after the deleted node
        delete q;
      } else {
      //if there isnt a node after this, just terminate the list and delete the node
        last->next = NULL;
        if(nodeptr == q)
          nodeptr = last; //set the current location to the node before the one deleted
        delete q;
      }
      return true;
    }
    last = q;
    q = p->next;
    p = q;
  }
  return false;
}

// Destructor de-allocates memory used by the list.
NodeList::~NodeList()
{
    node * p = head;
    node * q = head;
    while (q)
    {
        p = q;
        q = p -> next;
        if (q) delete p;
    }
}
