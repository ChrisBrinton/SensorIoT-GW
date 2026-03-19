//*******************************************************************
//  NodeList.h
//  NodeList_Project
//
//  Created by Karlina Beringer on June 12, 2014.
//  This header file contains the NodeList class declarations.
//*******************************************************************

#ifndef NodeList_h
#define NodeList_h

#include "Arduino.h"
#include "time.h"

#ifdef DEBUG_PORT
    #define DEBUG_MSG(...) DEBUG_PORT.printf( __VA_ARGS__ )
#else
    #define DEBUG_MSG(...)
#endif
#define DEBUG_NTPCLIENT

//*******************************************************************
// Node structs contain data and a pointer to the next node.
//*******************************************************************
struct node
{
    int nodeID;
    node* next;
    time_t tAge;
};

//*******************************************************************
// NodeList is a list of singly-linked nodes.
//*******************************************************************
class NodeList
{
private:
    // Head of the list contains no data,
    // but points to the list.
    node* head;
    int listLength;
    node* nodeptr;

public:
    // Default Constructor creates the head node.
    NodeList();

    // Setter adds a node to the list at a given position.
    // Takes a node and list position as parameters.
    // Position must be between 1 and the number of data nodes.
    // Returns true if the operation is successful.
    bool insertNode(int newNode);

    // Return true is a nodeID already exists in the list
    bool nodeExists(int nodeID);

    // Set the the last time the node was updated
    bool setUpdateTime(int nodeID, time_t tUpdate);

    // Return the node update age in minutes
    int getNodeUpdateAge(int nodeID);

    // Return the next item in the list
    int nextNode();

    // Return the current item in the list
    int currentNode();

    // Delete the node
    bool deleteNode(int nodeID);

    // Destructor de-allocates memory used by the list.
    ~NodeList();
};

#endif
