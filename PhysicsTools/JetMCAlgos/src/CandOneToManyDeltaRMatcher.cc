#include "PhysicsTools/JetMCAlgos/interface/CandOneToManyDeltaRMatcher.h"
#include "PhysicsTools/JetMCUtils/interface/CandMatchMapMany.h"

#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Utilities/interface/EDMException.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ParameterSet/interface/InputTag.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "DataFormats/Common/interface/Handle.h"
#include "DataFormats/Candidate/interface/Candidate.h"
#include "DataFormats/Candidate/interface/LeafCandidate.h"
#include "DataFormats/Candidate/interface/CandMatchMap.h"
#include "DataFormats/Candidate/interface/CandidateFwd.h"

#include <Math/VectorUtil.h>
#include <TMath.h>

using namespace edm;
using namespace std;
using namespace reco;
using namespace ROOT::Math::VectorUtil;

namespace reco {
  namespace helper {
    typedef pair<size_t, double> MatchPair;
    
    struct SortBySecond {
      bool operator()( const MatchPair & p1, const MatchPair & p2 ) const {
	return p1.second < p2.second;
      } 
    };
  }
}

CandOneToManyDeltaRMatcher::CandOneToManyDeltaRMatcher( const ParameterSet & cfg ) :
  source_( cfg.getParameter<InputTag>( "src" ) ),
  matched_( cfg.getParameter<InputTag>( "matched" ) ),
  printdebug_( cfg.getUntrackedParameter<bool>("printDebug", false) ) {
  produces<CandMatchMapMany>();
}

CandOneToManyDeltaRMatcher::~CandOneToManyDeltaRMatcher() {
}
		
void CandOneToManyDeltaRMatcher::produce( Event& evt, const EventSetup& es ) {
  
  Handle<CandidateCollection> source;  
  Handle<CandidateCollection> matched;  
  evt.getByLabel( source_, source ) ;
  evt.getByLabel( matched_, matched ) ;
 
  if (printdebug_) {
    for( CandidateCollection::const_iterator c = source->begin(); c != source->end(); ++c ) {
      cout << "[CandOneToManyDeltaRMatcher] Et source  " << c->et() << endl;
    }    
    for( CandidateCollection::const_iterator c = matched->begin(); c != matched->end(); ++c ) {
      cout << "[CandOneToManyDeltaRMatcher] Et matched " << c->et() << endl;
    } 
  }
 

  auto_ptr<CandMatchMapMany> matchMap( new CandMatchMapMany( CandMatchMapMany::ref_type( CandidateRefProd( source  ),
                                                                                         CandidateRefProd( matched )
											 ) ) );
  for( size_t c = 0; c != source->size(); ++ c ) {
    const Candidate & src = (*source)[ c ];
    if (printdebug_) cout << "[CandOneToManyDeltaRMatcher] source (Et,Eta,Phi) =(" << src.et() << "," << 
                                                                                      src.eta() << "," << 
		                                                                      src.phi() << ")" << endl;
    vector<helper::MatchPair> v;
    for( size_t m = 0; m != matched->size(); ++ m ) {
      const Candidate & match = ( * matched )[ m ];
      double dist = DeltaR( src.p4() , match.p4() );
      v.push_back( make_pair( m, dist ) );      
    }
    if ( v.size() > 0 ) {
      sort( v.begin(), v.end(), helper::SortBySecond() );
      for( size_t m = 0; m != v.size(); ++ m ) {
	if (printdebug_) cout << "[CandOneToManyDeltaRMatcher]       match (Et,Eta,Phi) =(" << ( * matched )[ m ].et() << "," << 
                                                                                               ( * matched )[ m ].eta() << "," << 
                                                                                               ( * matched )[ m ].phi() << ") DeltaR=" << 
			                                                                       v[m].second  << endl;
	matchMap->insert( CandidateRef( source, c ), make_pair( CandidateRef( matched, v[m].first ), v[m].second  )  );
      }    
    } 
  }
  
  evt.put( matchMap );

}

