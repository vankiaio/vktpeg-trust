/**
 * *  The apply() methods must have C calling convention so that the blockchain can lookup and
 * *  call these methods.
 * */
#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>
#include <string>

extern "C" {
	struct actioninfo {
	   uint64_t id;
	   account_name receiver;
	   account_name code;
	   account_name action;

	   uint64_t primary_key()const { return id; }

	   //EOSLIB_SERIALIZE( notifyinfo, (id)(receiver)(code)(action) )
	};

	typedef eosio::multi_index< N(actioninfo), actioninfo> actioninfo_index;

	struct dummy_action {
		account_name to;
		eosio::asset quantity;
	};

	struct dummyinfo {
	   uint64_t id;
	   account_name to;
	   eosio::asset quantity;
	   uint64_t primary_key()const { return id; }

	   //EOSLIB_SERIALIZE( gameinfo, (id)(player)(x)(y) )
	};

	typedef eosio::multi_index< N(dummyinfo), dummyinfo> dummyinfo_index;



    struct transfer_args {
       account_name  from;
       account_name  to;
       eosio::asset         quantity;
       std::string        memo;
    };

    struct transferinfo {
    	uint64_t id;
    	account_name from;
    	account_name to;
    	eosio::asset quantity;
    	std::string memo;

    	uint64_t primary_key() const {return id;}
    };

    typedef eosio::multi_index<N(transferinfo), transferinfo> tansferinfo_index;

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
		 transfer_args args = eosio::unpack_action_data<transfer_args>();
		 tansferinfo_index transferinfos(_self, _self);
		 auto new_tansferinfo_itr = transferinfos.emplace(_self, [&](auto& info){
			info.id           = transferinfos.available_primary_key();
			info.from         = args.from;
			info.to           = args.to;
			info.quantity     = args.quantity;
			info.memo     = args.memo;
		 });
	 }
	 //
	 if(code == N(pegzone) && action == N(transfer)) {
		 dummy_action dummy = eosio::unpack_action_data<dummy_action>();
		 dummyinfo_index dummyinfos(_self, _self);
		 auto new_dummyinfo_itr = dummyinfos.emplace(_self, [&](auto& info){
			info.id         = dummyinfos.available_primary_key();
			info.to    = dummy.to;
			info.quantity    = dummy.quantity;
		 });

         eosio::action(
            eosio::permission_level{ _self, N(active) },
            N(eosio.token), N(transfer),
            std::make_tuple(_self, dummy.to, dummy.quantity, std::string("xxx"))
         ).send();
	 }

	 //
     if(code == N(pegzone) && action == N(reset)) {
   	  //
    	 {
    		 actioninfo_index actioninfos(_self, _self);
			 auto actioninfos_itr = actioninfos.begin();
			 while(actioninfos_itr != actioninfos.end()) {
				 actioninfos_itr = actioninfos.erase(actioninfos_itr);
			 }
    	 }

    	 {
    		 tansferinfo_index transferinfos(_self, _self);
			 auto transferinfos_itr = transferinfos.begin();
			 while(transferinfos_itr != transferinfos.end()) {
				 transferinfos_itr = transferinfos.erase(transferinfos_itr);
			 }
    	 }

    	 {
    		 dummyinfo_index dummyinfos(_self, _self);
			 auto dummyinfos_itr = dummyinfos.begin();
			 while(dummyinfos_itr != dummyinfos.end()) {
				 dummyinfos_itr = dummyinfos.erase(dummyinfos_itr);
			 }
    	 }
     }
   }
} // extern "C"
