#define MAX_TRADE 5
#define MAX_PROD 5
#define  ASSIGNSHOP(shp_nr, fname) { if (real_shop(shp_nr) > 0) \
                           shop_index[real_shop(shp_nr)].shp_func = fname;}
struct shop_data {
    int         virtual;            /* virtual number of shop               */
    int 	producing[MAX_PROD];/* Which item to produce (virtual)      */
    float	profit_buy;         /* Factor to multiply cost with         */
    float	profit_sell;        /* Factor to multiply cost with.        */
    char	*no_such_item1;     /* Message if keeper hasn't got an item */
    char	*no_such_item2;     /* Message if player hasn't got an item */
    char	*missing_cash1;     /* Message if keeper hasn't got cash    */
    char	*missing_cash2;     /* Message if player hasn't got cash    */
    char	*do_not_buy;	    /* If keeper dosn't buy such things.    */
    char	*message_buy;       /* Message when player buys item        */
    char	*message_sell;      /* Message when player sells item       */
    int	        temper1;       	    /* How does keeper react if no money    */
    int 	temper2;            /* How does keeper react when attacked  */
    int         keeper;             /* The mobil who owns the shop (virtual)*/
    int         with_who;	    /* Who does the shop trade with?	    */
    int 	in_room;            /* Where is the shop?		    */
    int 	open1, open2;	    /* When does the shop open?		    */
    int 	close1, close2;	    /* When does the shop close?	    */
    int 	bankAccount;   	    /* Store all gold over 15000            */
    long        tradetype;          /* bitvector for object types           */
    int         (*shp_func)();      /* special function for shop            */
};  

