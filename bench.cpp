
#include <connectivitymatrix.h>
#include <iostream>
#include <cstdlib>

int main( int argc, char** argv) {
  std::srand( 0);
  
  ConnectivityMatrix cm;
  std::vector< ConnectivityMatrix::NodeId> nodes;
  size_t nNodes( 1000000);
  size_t nMatches( 10);
  size_t nRuns( 1);
  
  for( ConnectivityMatrix::NodeId id = 0; id < nNodes; ++id) {
    nodes.push_back( id);
  }

  for( size_t run = 0; run < nRuns; ++run) {
    for( size_t node = 0; node < nodes.size(); ++node) {
      std::vector< ConnectivityMatrix::NodeId> matches;
      matches.reserve( nMatches);

      for( size_t match = 0; match < nMatches; ++match) {
        // extreme case - constant reclustering
        //matches.push_back( nodes[ std::rand() % nodes.size()]);

        // results in one big cluster of consecutive matches
        //matches.push_back( (node + match) % nNodes);

        // clusters of nMatches
        matches.push_back( ( node - ( node % nMatches)) + match);
      }

      std::sort( matches.begin(), matches.end());
      matches.erase( std::unique( matches.begin(), matches.end()), matches.end());

      //std::cout << node << "\n";
      cm.updateMatches( nodes[ node], matches);
    }
  }
}
