#include <vector>
#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <iterator>
#include <cassert>

template< typename T>
class SymmetricMatchMatrix {
public:
  typedef T value_type;
  
  void setMatches
  ( const value_type& probe, std::vector< value_type> updatedMatches) {
    assert( std::is_sorted( updatedMatches.begin(), updatedMatches.end()));
    assert
      ( std::adjacent_find
        ( updatedMatches.begin(), updatedMatches.end()) ==
        updatedMatches.end());
    
    auto& outdatedMatches( matches[ probe]);

    removeMatches( getRemovedMatches( updatedMatches, outdatedMatches), probe);
    insertMatches( getNewMatches( updatedMatches, outdatedMatches), probe);
    
    if( updatedMatches.empty()) {
      matches.erase( probe);
    } else {
      outdatedMatches.swap( updatedMatches);
    }
  }

  const std::vector< value_type>& getMatches( const value_type& probe) const {
    auto matchesIt( matches.find( probe));

    if( matchesIt == matches.end()) {
      return emptyMatches;
    } else {
      return matchesIt->second;
    }
  }

  std::vector< value_type> getProbes() const {
    std::vector< value_type> probes;
    probes.reserve( matches.size());

    for( auto& matchesOfProbe : matches) {
      probes.push_back( matchesOfProbe.first);
    }

    std::sort( probes.begin(), probes.end());

    return probes;
  }

	std::vector< value_type> getCluster( const value_type& probe) const {
    if( matches.find( probe) != matches.end()) {
      std::unordered_set< value_type> clustered;
      std::stack< value_type> toProcess;

      toProcess.push( probe);

      while( !toProcess.empty()) {
        auto match( toProcess.top());
        toProcess.pop();

        if( clustered.insert( match).second) {
          for( auto& matchOfMatch : matches.find( match)->second) {
            if( clustered.find( matchOfMatch) == clustered.end()) {
              toProcess.push( matchOfMatch);
            }
          }
        }
      }

      std::vector< value_type> cluster;
      cluster.reserve( clustered.size());
    
      for( auto& match : clustered) {
        cluster.push_back( std::move( match));
      }

      std::sort( cluster.begin(), cluster.end());

      return cluster;
    } else {
      return std::vector< value_type>();
    }
	}

  void clear() {
    matches.clear();
  }
private:
  std::vector< value_type> getNewMatches
  ( const std::vector< value_type>& updatedMatches,
    const std::vector< value_type>& outdatedMatches) const {
    std::vector< value_type> newMatches;
    newMatches.reserve( updatedMatches.size());

    set_difference
      ( updatedMatches.begin(),
        updatedMatches.end(),
        outdatedMatches.begin(),
        outdatedMatches.end(),
        std::back_inserter( newMatches));

    return newMatches;
  }

  std::vector< value_type> getRemovedMatches
  ( const std::vector< value_type>& updatedMatches,
    const std::vector< value_type>& outdatedMatches) const {
    std::vector< value_type> removedMatches;
    removedMatches.reserve( outdatedMatches.size());

    set_difference
      ( outdatedMatches.begin(),
        outdatedMatches.end(),
        updatedMatches.begin(),
        updatedMatches.end(),
        std::back_inserter( removedMatches));
    
    return removedMatches;
  }

  void insertMatches
  ( const std::vector< value_type>& newMatches,
    const value_type& probe) {
    for( auto& newMatch : newMatches) {
      auto& matchesOfNewMatch( matches[ newMatch]);
      auto probeIt
        ( std::lower_bound
          ( matchesOfNewMatch.begin(), matchesOfNewMatch.end(), probe));

      matchesOfNewMatch.insert( probeIt, probe);
    }
  }
    
  void removeMatches
  ( const std::vector< value_type>& removedMatches,
    const value_type& probe) {
    for( auto& removedMatch : removedMatches) {
      auto& matchesOfRemovedMatch( matches[ removedMatch]);
      auto probeIt
        ( std::lower_bound
          ( matchesOfRemovedMatch.begin(), matchesOfRemovedMatch.end(), probe));

      matchesOfRemovedMatch.erase( probeIt);

      if( matchesOfRemovedMatch.empty()) {
        matches.erase( removedMatch);
      }
    }
  }
  
  std::unordered_map< value_type, std::vector< value_type> > matches;
  std::vector< value_type> emptyMatches;
};
