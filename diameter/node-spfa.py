class Graph:
    def __init__(self):
        print "Graph inited"
        self._nodes = dict()

    def getOrCreateNode(self, nodeId):
        if (not nodeId in self._nodes):
            self._nodes[nodeId] = GraphNode(nodeId)
        return self._nodes[nodeId]

    def getNodes(self):
        return self._nodes

    def runBelmanFord(self):
        firstNodeTuple = self._nodes.iteritems().next()
        firstNode = firstNodeTuple[1]
        hdrs = ""
        for nodeId, node in self._nodes.iteritems():
            hdrs += "\t{0}".format(nodeId)
        print "Distance from {0} to...".format(firstNode.nodeId())
        print hdrs
        for k in xrange(0, len(self._nodes)):
            s = ""
            for nodeId, node in self._nodes.iteritems():
                node.propagateDistance(firstNode)
            for nodeId, node in self._nodes.iteritems():
                node.commitDistanceRun(firstNode)
                s += "\t{0}".format(node.getDistance(firstNode))
            print s

class GraphNode:
    def __init__(self, nodeId):
        self._nodeId = nodeId
        self._connectedNodes = dict()
        self._distances = dict()
        self._thisRunDistance = None

    def connectTo(self, other, distance):
        if (other in self._connectedNodes):
            assert(distance == self._connectedNodes[other])
        else:
            self._connectedNodes[other] = distance

    def connectedNodes(self):
        return self._connectedNodes

    def nodeId(self):
        return self._nodeId

    def updateDistance(self, fromNode, destNode, distance):
        assert(fromNode in self._connectedNodes)
        if (distance is None):
            return
        if (self == destNode):
            return

        updateDistance = distance + self._connectedNodes[fromNode]
        if (None == self._thisRunDistance):
            self._thisRunDistance = updateDistance
        else:
            if (self._thisRunDistance > updateDistance):
                self._thisRunDistance = updateDistance

    def commitDistanceRun(self, destNode):
        if (self._thisRunDistance is None):
            return

        if (destNode in self._distances):
            if (self._thisRunDistance < self._distances[destNode]):
                self._distances[destNode] = self._thisRunDistance
        else:
            self._distances[destNode] = self._thisRunDistance
        self._thisRunDistance = None

    def propagateDistance(self, destNode):
        if (destNode == self):
            dist = 0
        elif (destNode in self._distances):
            dist = self._distances[destNode];
        else:
            dist = None;
        for node, weight in self._connectedNodes.iteritems():
            node.updateDistance(self, destNode, dist)

    def getDistance(self, destNode):
        if (destNode in self._distances):
            return self._distances[destNode]
        return None

import sys
if (2 != len(sys.argv)):
    print "Usage: {0} <file-path>".format(sys.argv[0])
    sys.exit(1)

graph = Graph();
f = open(sys.argv[1]);
for line in f:
    tokens = line.split()
    assert(3 == len(tokens))
    node = graph.getOrCreateNode(tokens[0])
    outNode = graph.getOrCreateNode(tokens[1])
    node.connectTo(outNode, int(tokens[2]))
    outNode.connectTo(node, int(tokens[2]))

graph.runBelmanFord()
