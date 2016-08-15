#include <symmetricmatchmatrix.h>

#include <vector>
#include <unordered_map>
#include <algorithm>
#include <iterator>

class ConnectivityMatrix {
public:
  typedef size_t ClusterId;
  typedef size_t NodeId;

  struct ClusterUpdate {
    std::vector< NodeId> newMatches;
    std::vector< NodeId> removedMatches;
  };

  struct ClusteringUpdate {
    std::unordered_map< ClusterId, ClusterUpdate> clusterUpdates;
    std::vector< ClusterId> removedClusters;
  };

  ConnectivityMatrix()
    : maxClusterId( 0) {
  }

  void init
  ( const std::vector< std::pair< ClusterId, std::vector< NodeId> > >&
    clustersOfClusters) {
    clear();
    
    for( auto& clusterOfCluster : clustersOfClusters) {
      auto& clusterId( clusterOfCluster.first);
      auto& cluster( clusterOfCluster.second);

      initCluster( clusterId, cluster);
      
      maxClusterId = std::max( maxClusterId, clusterId);
    }
  }

  ClusteringUpdate updateMatches
  ( NodeId probe, const std::vector< NodeId>& updatedMatches) {
    matchMatrix.setMatches( probe, updatedMatches);

    auto updatedCluster( matchMatrix.getCluster( probe));

    return updateClustering( probe, updatedCluster);
  }

  std::vector< NodeId> getProbes() const {
    return matchMatrix.getProbes();
  }

  std::vector< ClusterId> getClusters() const {
    std::vector< ClusterId> clusters;
    clusters.reserve( clusterIdToCluster.size());

    for( auto& cluster : clusterIdToCluster) {
      clusters.push_back( cluster.first);
    }

    std::sort( clusters.begin(), clusters.end());

    return clusters;
  }
  
  const std::vector< NodeId>& getCluster( const ClusterId& cluster) const {
    auto clusterIt( clusterIdToCluster.find( cluster));

    if( clusterIt == clusterIdToCluster.end()) {
      return emptyCluster;
    } else {
      return clusterIt->second;
    }
  }
  
  void clear() {
    nodeIdToClusterId.clear();
    clusterIdToCluster.clear();
    maxClusterId = 0;
    matchMatrix.clear();
  }
private:
  void
  initCluster( const ClusterId& clusterId, const std::vector< NodeId>& cluster) {
    if( !cluster.empty()) {
      clusterIdToCluster[ clusterId] = cluster;
      setCluster( cluster, clusterId);
      matchMatrix.setMatches( cluster.front(), cluster);
    }
  }

  ClusteringUpdate
  updateClustering(const NodeId& probe, std::vector< NodeId>& updatedCluster) {
    auto lowestClusteredNodeIt
      ( std::find_if
        ( updatedCluster.begin(),
          updatedCluster.end(),
          [&]( NodeId node) {
          return nodeIdToClusterId.find( node) != nodeIdToClusterId.end();}));

    if( lowestClusteredNodeIt == updatedCluster.end()) {
      return updateClusteringForUnclusteredNodes( probe, updatedCluster);
    } else {
      ClusterId clusterId( nodeIdToClusterId[ *lowestClusteredNodeIt]);

      return updateClusteringForClusteredNodes( clusterId, updatedCluster);
    }
  }

  ClusteringUpdate updateClusteringForUnclusteredNodes
  ( const NodeId& probe, std::vector< NodeId>& updatedCluster) {
    if( updatedCluster.empty()) {
      return updateClusteringForRemovedProbe( probe);
    } else {
      return updateClusteringForNewNodes( updatedCluster);
    }
  }

  ClusteringUpdate updateClusteringForRemovedProbe( const NodeId& probe) {
    ClusteringUpdate clusteringUpdate;
    auto clusterOfProbeIt( nodeIdToClusterId.find( probe));
        
    if( clusterOfProbeIt != nodeIdToClusterId.end()) {
      ClusterId removedClusterId( clusterOfProbeIt->second);
      
      clusteringUpdate.removedClusters = { removedClusterId};
      clusterIdToCluster.erase( removedClusterId);
      nodeIdToClusterId.erase( clusterOfProbeIt);
    }

    return clusteringUpdate;
  }

  ClusteringUpdate updateClusteringForNewNodes
  ( std::vector< NodeId>& updatedCluster) {
    ClusteringUpdate clusteringUpdate;
    ClusterId newClusterId( createCluster( updatedCluster));

    clusteringUpdate.clusterUpdates[ newClusterId].newMatches.swap
      ( updatedCluster);

    return clusteringUpdate;
  }

  ClusteringUpdate updateClusteringForClusteredNodes
  ( const ClusterId& clusterId, std::vector< NodeId>& updatedCluster) {
    ClusteringUpdate clusteringUpdate;
    auto& outdatedCluster( clusterIdToCluster[ clusterId]);
    auto newNodes( getNewNodes( updatedCluster, outdatedCluster));
    auto removedNodes( getRemovedNodes( updatedCluster, outdatedCluster));
    std::vector< ClusterId> removedClusters( getUniqueClusters( newNodes));

    eraseClusters( removedClusters);

    clusteringUpdate.removedClusters.swap( removedClusters);

    for( auto newCluster : reclusterNodes( removedNodes)) {
      clusteringUpdate.clusterUpdates[ newCluster].newMatches =
        clusterIdToCluster[  newCluster];
    }
      
    clusteringUpdate.clusterUpdates[ clusterId].newMatches.swap( newNodes);
    clusteringUpdate.clusterUpdates[ clusterId].removedMatches.swap
      ( removedNodes);

    setCluster( newNodes, clusterId);
    outdatedCluster.swap( updatedCluster);

    return clusteringUpdate;
  }

  std::vector< NodeId> getNewNodes
  ( const std::vector< NodeId>& updatedNodes,
    const std::vector< NodeId>& outdatedNodes) const {
    std::vector< NodeId> newNodes;
    newNodes.reserve( updatedNodes.size());

    set_difference
      ( updatedNodes.begin(),
        updatedNodes.end(),
        outdatedNodes.begin(),
        outdatedNodes.end(),
        back_inserter( newNodes));

    return newNodes;
  }

  std::vector< NodeId> getRemovedNodes
  ( const std::vector< NodeId>& updatedNodes,
    const std::vector< NodeId>& outdatedNodes) const {
    std::vector< NodeId> removedNodes;
    removedNodes.reserve( outdatedNodes.size());

    set_difference
      ( outdatedNodes.begin(),
        outdatedNodes.end(),
        updatedNodes.begin(),
        updatedNodes.end(),
        back_inserter( removedNodes));

    return removedNodes;
  }

  std::vector< ClusterId>
  getUniqueClusters( const std::vector< NodeId>& nodes) const {
    std::vector< ClusterId> clusters;
    clusters.reserve( nodes.size());

    for( auto node : nodes) {
      auto clusterOfNodeIt( nodeIdToClusterId.find( node));

      if( clusterOfNodeIt != nodeIdToClusterId.end()) {
        clusters.push_back( clusterOfNodeIt->second);
      }
    }

    std::sort( clusters.begin(), clusters.end());
    clusters.erase( std::unique( clusters.begin(), clusters.end()), clusters.end());
      
    return clusters;
  }

  void eraseClusters( const std::vector< ClusterId>& clusters) {
    for( auto cluster : clusters) {
      clusterIdToCluster.erase( cluster);
    }
  }

  void setCluster( const std::vector< NodeId>& nodes, ClusterId clusterId) {
    for( auto node : nodes) {
      nodeIdToClusterId[ node] = clusterId;
    }
  }

  std::vector< ClusterId> reclusterNodes( std::vector< NodeId> nodes) {
    std::vector< std::vector< NodeId> > newClusters;
    newClusters.reserve( nodes.size());

    while( !nodes.empty()) {
      auto cluster( matchMatrix.getCluster( nodes.back()));

      if( cluster.empty()) {
        nodeIdToClusterId.erase( nodes.back());
        nodes.pop_back();
      } else {
        auto nodesWithoutClusteredNodes( getRemovedNodes( cluster, nodes)); 

        nodes.swap( nodesWithoutClusteredNodes);

        newClusters.push_back( std::move( cluster));
      }
    }

    std::vector< ClusterId> newClusterIds;
    newClusterIds.reserve( newClusters.size());

    for( auto& newCluster : newClusters) {
      newClusterIds.push_back( createCluster( std::move( newCluster)));
    }

    return newClusterIds;
  }

  ClusterId createCluster( std::vector< NodeId> cluster) {
    ClusterId newClusterId( createClusterId());

    setCluster( cluster, newClusterId);
    clusterIdToCluster[ newClusterId].swap( cluster);

    return newClusterId;
  }

  ClusterId createClusterId() {
    return ++maxClusterId;
  }
  
  SymmetricMatchMatrix< NodeId> matchMatrix;
  std::unordered_map< NodeId, ClusterId> nodeIdToClusterId;
  std::unordered_map< ClusterId, std::vector< NodeId> > clusterIdToCluster;
  size_t maxClusterId;
  const std::vector< NodeId> emptyCluster;
};
