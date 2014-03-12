class Graph:
    def __init__(self):
        print "Graph inited"
        self._nodes = dict();

    def getOrCreateNode(self, nodeId):
        if (not nodeId in self._nodes):
            self._nodes[nodeId] = GraphNode(nodeId)
        return self._nodes[nodeId];

    def getNodes(self):
        return self._nodes

    def runBelmanFord(self):
        firstNodeTuple = self._nodes.iteritems().next()
        firstNode = firstNodeTuple[1]
        s = ""
        for nodeId, node in self._nodes.iteritems():
            node.propagateDistance(firstNode)
        for nodeId, node in self._nodes.iteritems():
            s += "\t{0}".format(node.getDistance(firstNode))
        print s

class GraphNode:
    def __init__(self, nodeId):
        self._nodeId = nodeId
        self._connectedNodes = dict()
        self._distances = dict()

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

        updateDistance = distance + self._connectedNodes[fromNode]
        if (not destNode in self._distances):
            self._distances[destNode] = updateDistance
        else:
            thisDistance = self._distances[destNode]
            if (updateDistance < thisDistance):
                self._distances[destNode] = updateDistance

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

def main():
    gn = GraphNode();

graph = Graph();
f = open("./nodes.txt");
for line in f:
    tokens = line.split()
    assert(3 == len(tokens))
    node = graph.getOrCreateNode(tokens[0])
    outNode = graph.getOrCreateNode(tokens[1])
    node.connectTo(outNode, int(tokens[2]))

graph.runBelmanFord()
