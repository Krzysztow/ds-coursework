import collections


class Graph:
    def __init__(self):
        self._nodes = collections.OrderedDict()

    def getOrCreateNode(self, nodeId):
        if (not nodeId in self._nodes):
            self._nodes[nodeId] = GraphNode(nodeId)
        return self._nodes[nodeId]

    def getNodes(self):
        return self._nodes


class GraphNode:
    def __init__(self, nodeId):
        self._nodeId = nodeId
        self._connectedNodes = dict()
        self._distances = dict()
        self._tag = None

    def setTag(self, tagData):
        self._tag = tagData

        self._lastRunDistance = None
        self._thisRunDistance = None
        self._lastShortPathSrc = None

    def getTag(self):
        return self._tag

    def connectTo(self, other, distance):
        if (other in self._connectedNodes):
            assert(distance == self._connectedNodes[other])
        else:
            self._connectedNodes[other] = distance

    def connectedNodes(self):
        return self._connectedNodes

    def nodeId(self):
        return self._nodeId


class BFAlgorithm:
    def __init__(self, graph):
        self._graph = graph
        self._debug = False

    def setNodesTagData(self, rootNode):
        for nodeId, node in self._graph.getNodes().iteritems():
            node.setTag(BFData(node))

        for nodeId, node in self._graph.getNodes().iteritems():
            node.getTag().beginAlg(rootNode)

    def nodes(self):
        return self._graph.getNodes();

    def createResult(self):
        res = dict();
        for nodeId, node in self.nodes().iteritems():
            bfData = node.getTag()
            res[nodeId] = (bfData._lastRunDistance, bfData._lastRunPath)

        return res

    def runBelmanFord(self, rootNodeId):
    	rootNode = self.nodes()[rootNodeId]

        self.setNodesTagData(rootNode)

        if (self._debug):
            self.printHeaders(rootNode)
            self.printValues()

        for k in xrange(0, len(self._graph.getNodes())):
            for nodeId, node in self.nodes().iteritems():
                node.getTag().propagateDistance()
            for nodeId, node in self.nodes().iteritems():
                node.getTag().finishRun()
            if (self._debug):
                self.printValues()

        if (self._debug):
            self.printPaths()

        #TODO: delete tag data along with creating results
        return self.createResult()

    def printHeaders(self, refNode):
        hdrs = ""
        for nodeId, node in self.nodes().iteritems():
            hdrs += "\t{0}".format(nodeId)
        print "Distance from {0} to...".format(refNode.nodeId())
        print hdrs

    def printValues(self):
        s = ""
        for nodeId, node in self.nodes().iteritems():
            s += "\t{0}".format(node.getTag().getLastDistance())
        print s

    def printPaths(self):
        for nodeId, node in self.nodes().iteritems():
            print node.getTag()._lastRunPath

class BFData:
    def __init__(self, ownerNode):
        self._node = ownerNode
        self._lastRunDistance = None
        self._lastRunPath = None
        self._thisRunDistance = None
        self._thisRunPath = None
        self._lastShortPathSrc = None

    def beginAlg(self, destNode):
        self._lastShortPathSrc = None
        if (self._node == destNode):
            self._lastRunDistance = 0
        else:
            self._lastRunDistance = None;
        self._thisRunDistance = self._lastRunDistance

        self._lastRunPath = self._node.nodeId()
        self._thisRunPath = self._lastRunPath

    def finishRun(self):
        self._lastRunDistance = self._thisRunDistance
        self._lastRunPath = self._thisRunPath + "->" + self._node.nodeId()

    def updateDistance(self, fromData, distance):
        assert(not distance is None)
        if (None == self._thisRunDistance):
            self._thisRunDistance = distance
            self._lastShortPathSrc = fromData._node
            self._thisRunPath = fromData._lastRunPath
        else:
            if (self._thisRunDistance > distance):
                self._thisRunDistance = distance
                self._lastShortPathSrc = fromData._node
                self._thisRunPath = fromData._lastRunPath

    def propagateDistance(self):
        if (self._lastRunDistance is None):
            return
        for node, weight in self._node.connectedNodes().iteritems():
            if (node == self._lastShortPathSrc):
                '''print "{0} skipping propagating to {1}".format(self.nodeId(), node.nodeId())'''
            else:
                node.getTag().updateDistance(self, self._lastRunDistance + weight)

    def getLastDistance(self):
        return self._lastRunDistance

def createGraphFromFile(filePath):
    graph = Graph();
    f = open(sys.argv[1])
    for line in f:
        tokens = line.split()
        assert(3 == len(tokens))
        node = graph.getOrCreateNode(tokens[0])
        outNode = graph.getOrCreateNode(tokens[1])
        node.connectTo(outNode, int(tokens[2]))
    return graph

import sys

if (3 != len(sys.argv)):
    print "Usage: {0} <file-path> root".format(sys.argv[0])
    sys.exit(1)

graph = createGraphFromFile(sys.argv[1])
alg = BFAlgorithm(graph)

longestPaths = dict();
for nodeId, node in graph.getNodes().iteritems():
    print "-- Runnig BF for " + nodeId + " --"
    res = alg.runBelmanFord(nodeId)

    for resNodeId, resTuple in res.iteritems():
        print "from {0} to {1} dist={2} : {3}".format(nodeId, resNodeId, resTuple[0], resTuple[1])

    #find the longest path for given root
    longstResTuple = res.iteritems().next()[1];
    for resNodeId, resTuple in res.iteritems():
        if (longstResTuple[0] < resTuple[0]):
            longstResTuple = resTuple
    longestPaths[nodeId] = longstResTuple
    print "LONGEST: dist: {0} path: {1}".format(longstResTuple[0], longstResTuple[1])

#find max for the longest path globally - diameter
#NOTE: I could have done it at once, but it serves my debugging purposes
diameter = longestPaths.iteritems().next()[1]
for rootNodeId, resTuple in longestPaths.iteritems():
    if (diameter[0] < resTuple[0]):
        diameter = resTuple

print "=========================================================="
print "DIAMETER: D={0} ({1}).".format(diameter[0], diameter[1])
