import collections

class Graph:
    def __init__(self):
        print "Graph inited"
        self._nodes = collections.OrderedDict()

    def getOrCreateNode(self, nodeId):
        if (not nodeId in self._nodes):
            self._nodes[nodeId] = GraphNode(nodeId)
        return self._nodes[nodeId]

    def getNodes(self):
        return self._nodes

    def runBelmanFord(self, nodeId):
    	firstNode = self._nodes[nodeId];
        for nodeId, node in self._nodes.iteritems():
            node.beginAlg(firstNode)
        self.printHeaders(firstNode);
        self.printValues();
        for k in xrange(0, len(self._nodes)):
            for nodeId, node in self._nodes.iteritems():
                node.propagateDistance(firstNode)
            for nodeId, node in self._nodes.iteritems():
                node.finishRun()
            self.printValues();

    def printHeaders(self, refNode):
        hdrs = ""
        for nodeId, node in self._nodes.iteritems():
            hdrs += "\t{0}".format(nodeId)
        print "Distance from {0} to...".format(refNode.nodeId())
        print hdrs

    def printValues(self):
        s = ""
        for nodeId, node in self._nodes.iteritems():
            s += "\t{0}".format(node.getLastDistance())
        print s

class GraphNode:
    def __init__(self, nodeId):
        self._nodeId = nodeId
        self._connectedNodes = dict()
        self._distances = dict()

        self._lastRunDistance = None
        self._thisRunDistance = None
        self._lastShortPathSrc = None

    def connectTo(self, other, distance):
        if (other in self._connectedNodes):
            assert(distance == self._connectedNodes[other])
        else:
            self._connectedNodes[other] = distance

    def connectedNodes(self):
        return self._connectedNodes

    def nodeId(self):
        return self._nodeId

    def beginAlg(self, destNode):
        self._lastShortPathSrc = None
        if (self == destNode):
            self._lastRunDistance = 0
        elif (destNode in self._distances):
            self._lastRunDistance = self._distances[destNode]
        else:
            self._lastRunDistance = None;
        self._thisRunDistance = self._lastRunDistance

    def finishRun(self):
        self._lastRunDistance = self._thisRunDistance

    def updateDistance(self, fromNode, distance):
        assert(not distance is None)

        if (None == self._thisRunDistance):
            self._thisRunDistance = distance
            self._lastShortPathSrc = fromNode
        else:
            if (self._thisRunDistance > distance):
                self._thisRunDistance = distance
                self._lastShortPathSrc = fromNode

    def propagateDistance(self, destNode):
        if (self._lastRunDistance is None):
            return
        for node, weight in self._connectedNodes.iteritems():
            if (node == self._lastShortPathSrc):
                '''print "{0} skipping propagating to {1}".format(self.nodeId(), node.nodeId())'''
            else:
                node.updateDistance(self, self._lastRunDistance + weight)

    def getLastDistance(self):
        return self._lastRunDistance;

import sys
if (3 != len(sys.argv)):
    print "Usage: {0} <file-path> root".format(sys.argv[0])
    sys.exit(1)

graph = Graph();
f = open(sys.argv[1]);
for line in f:
    tokens = line.split()
    assert(3 == len(tokens))
    node = graph.getOrCreateNode(tokens[0])
    outNode = graph.getOrCreateNode(tokens[1])
    node.connectTo(outNode, int(tokens[2]))

graph.runBelmanFord(sys.argv[2])
