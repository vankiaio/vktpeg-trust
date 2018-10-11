/**
 * *  The apply() methods must have C calling convention so that the blockchain can lookup and
 * *  call these methods.
 * */
#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>
#include <string>

extern "C" {
	struct actioninfo {
	   uint64_t         id;
	   account_name     receiver;
	   account_name     code;
	   account_name     action;
	   uint64_t primary_key()const { return id; }
	};
	typedef eosio::multi_index< N(actioninfo), actioninfo> actioninfo_index;

    struct eostranargs {
       account_name      from;
       account_name      to;
       eosio::asset      quantity;
       std::string       memo;
    };
    struct eostransfer {
    	uint64_t         id;
    	account_name     from;
    	account_name     to;
    	eosio::asset     quantity;
    	std::string      memo;
    	uint64_t primary_key() const {return id;}
    };
    typedef eosio::multi_index<N(eostransfer), eostransfer> eostransfer_index;


	struct sidetranargs {
		std::string      from;
		account_name     to;
		eosio::asset     quantity;
		std::string      memo;
		uint64_t         index;
	};

	struct sidetransfer {
	   uint64_t          id;
	   std::string       from;
	   account_name      to;
	   eosio::asset      quantity;
	   std::string       memo;
	   uint64_t          index;
	   uint64_t primary_key()const { return id; }
	};
	typedef eosio::multi_index< N(sidetransfer), sidetransfer> sidetransfer_index;

	struct sidereqcount {
	   uint64_t          id;
	   uint64_t          g_index;
	   uint64_t primary_key()const { return id; }
	};
	typedef eosio::multi_index< N(sidereqcount), sidereqcount> sidereqcount_index;

	struct sidereqerror {
	   uint64_t          id;
	   uint64_t          index;
	   std::string       error;
	   uint64_t primary_key()const { return id; }
	};
	typedef eosio::multi_index< N(sidereqerror), sidereqerror> sidereqerror_index;


   /// The apply method implements the dispatch of events to this contract
   void apply( uint64_t receiver, uint64_t code, uint64_t action ) {
	 auto _self = receiver;
	 {
		 actioninfo_index actioninfos(_self, _self);
		 auto new_actioninfo_itr = actioninfos.emplace(_self, [&](auto& info){
			info.id           = actioninfos.available_primary_key();
			info.receiver     = receiver;
			info.code         = code;
			info.action       = action;
		 });
	 }

	 //
	 if(code == N(eosio.token) && action == N(transfer)) {
		 eostranargs args = eosio::unpack_action_data<eostranargs>();
		 eostransfer_index eostransfers(_self, _self);
		 auto new_eostansfer_itr = eostransfers.emplace(_self, [&](auto& info){
			info.id           = eostransfers.available_primary_key();
			info.from         = args.from;
			info.to           = args.to;
			info.quantity     = args.quantity;
			info.memo         = args.memo;
		 });
	 }

	 //
	 if(code == N(pegzone) && action == N(transfer)) {
		 sidetranargs args = eosio::unpack_action_data<sidetranargs>();
		 sidetransfer_index sidetransfers(_self, _self);
		 auto new_sidetransfer_itr = sidetransfers.emplace(_self, [&](auto& info){
			info.id         = sidetransfers.available_primary_key();
			info.from       = args.from;
			info.to         = args.to;
			info.quantity   = args.quantity;
			info.memo       = args.memo;
			info.index      = args.index;
		 });

		 sidereqcount_index sidereqcount(_self, _self);
		 auto cur_sidereqcount_itr = sidereqcount.find(_self);
		 if(cur_sidereqcount_itr == sidereqcount.end()) {
			 cur_sidereqcount_itr = sidereqcount.emplace(_self, [&](auto& info){
				 info.id       = sidereqcount.available_primary_key();
				 info.g_index  = 0;
			 });
		 }

		 if(new_sidetransfer_itr->index != cur_sidereqcount_itr->g_index) {
			 sidereqerror_index sidereqerrors(_self, _self);
			 auto new_sidereqcount_itr = sidereqerrors.emplace(_self, [&](auto& info){
				 info.id       = sidereqerrors.available_primary_key();
				 info.index    = new_sidetransfer_itr->index;
				 info.error    = "request sequence is invalid";
			 });
			 return;
		 }

         eosio::action(
            eosio::permission_level{ _self, N(active) },
            N(eosio.token), N(transfer),
            std::make_tuple(_self, args.to, args.quantity, args.memo)
         ).send();

         //
         sidereqcount.modify(cur_sidereqcount_itr, 0, [&](auto& info){
        	 info.g_index ++;
         });
	 }

	 //
     if(code == N(pegzone) && action == N(reset)) {
    	 {
    		 actioninfo_index actioninfos(_self, _self);
			 auto actioninfos_itr = actioninfos.begin();
			 while(actioninfos_itr != actioninfos.end()) {
				 actioninfos_itr = actioninfos.erase(actioninfos_itr);
			 }
    	 }

    	 {
    		 eostransfer_index eostransfers(_self, _self);
			 auto eostransfers_itr = eostransfers.begin();
			 while(eostransfers_itr != eostransfers.end()) {
				 eostransfers_itr = eostransfers.erase(eostransfers_itr);
			 }
    	 }

    	 {
    		 sidetransfer_index sidetransfers(_self, _self);
			 auto sidetransfers_itr = sidetransfers.begin();
			 while(sidetransfers_itr != sidetransfers.end()) {
				 sidetransfers_itr = sidetransfers.erase(sidetransfers_itr);
			 }
    	 }

    	 {
    		 sidereqcount_index sidereqcounts(_self, _self);
			 auto sidereqcounts_itr = sidereqcounts.begin();
			 while(sidereqcounts_itr != sidereqcounts.end()) {
				 sidereqcounts_itr = sidereqcounts.erase(sidereqcounts_itr);
			 }
    	 }

    	 {
    		 sidereqerror_index sidereqerrors(_self, _self);
			 auto sidereqerrors_itr = sidereqerrors.begin();
			 while(sidereqerrors_itr != sidereqerrors.end()) {
				 sidereqerrors_itr = sidereqerrors.erase(sidereqerrors_itr);
			 }
    	 }
     }
   }
} // extern "C"
