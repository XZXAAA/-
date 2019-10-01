#include "douyuAPI.h"
#include "stdio.h"
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>
#include "data.h"
#include <algorithm>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

void * keep_heart (void *handle)
{
    int ret =0;
    struct timeval val;
    while(1)
    {
        gettimeofday(&val,NULL);
        ret =  keep_heart(handle,val.tv_sec);
        sleep(45);     
    }
}

class findvalue
{
    public:
    findvalue(string s):val(s)
    {

    }
    bool operator()(const struct key_value value)
    {
        if(value.key == val)
        return 1 ;

        return 0;
    }

    private:
    string val;
};



int main(int argc, char * argv[])
{
   int ret =0 ; 
    void *handle = NULL;
   vector<key_value> data;
    vector<key_value>::iterator it;
    vector<key_value>::iterator it2;
    DY_MESSAGE_TYPE type;

    if(argc != 2)
    {
        printf("usage err\n");
        printf("使用方法 ：带一个房间号参数\n");
        return -1;
    }
   int room_id = atoi(argv[1]);

    ret = douyusockt_init(&handle,2,2,2);
    if(ret !=0)
    {
        printf("douyusockt_init err ret %d \n",ret);
        return -1;
    }
     struct hostent * host = gethostbyname("openbarrage.douyutv.com");
    //ret = connect_douyu(handle,"124.95.155.51",room_id);///接入服务器   ///通过ret == -1 和timeOUT可以判断超时
    ret = connect_douyu(handle,inet_ntoa(*(struct in_addr *)host->h_addr_list[0]),8601);//1126960
    struct in_addr ad;
    if(ret !=0)
    {
        printf("connect_douyu err ret %d \n",ret);
        return -1;
    }

   
    ret = login_room(handle);
	if(0 != ret)
	{
         printf("login_room err ret %d \n",ret);
		return -1;
	}

    ret = rev_data(handle,data,&type);
    if(type = MSG_TYPE_LOGIN_RESPONSE)
    {
        it =find_if(data.begin(),data.end(),findvalue("live_stat"));
            printf("直播间状态%s \n",(*it).value.c_str());
    }


    ret = join_room(handle,room_id, -9999);
	if(0 != ret)
	{
        printf("join_room err ret %d \n",ret);
		return -1;
	}






    pthread_t tid = 0;
    ret = pthread_create(&tid,NULL,&keep_heart,handle);
        if(ret !=0)
    {
        printf("pthread_create err ret %d \n",ret);
        return -1;
    }






    while(1)
    {
        ret = rev_data(handle,data,&type);   //根据收到数据类型switch

        switch (type)
        {
        case MSG_TYPE_BARRAGE:
            /* code */
            it =find_if(data.begin(),data.end(),findvalue("txt"));
           it2 =find_if(data.begin(),data.end(),findvalue("nn"));
            printf(" %s :\t\t%s \n",(*it2).value.c_str(),(*it).value.c_str());
            break;
        
        case MSG_TYPE_LOGIN_RESPONSE:
            it =find_if(data.begin(),data.end(),findvalue("live_stat"));
            printf("%s \n",(*it).value.c_str());

        break;
        default:
            break;
        }
        data.clear();
    }



    ret = pthread_join(tid,NULL);
    if(ret !=0)
    {
        printf("pthread_join err ret %d \n",ret);
        return -1;
    }
    return 0;

}