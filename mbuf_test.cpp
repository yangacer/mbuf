#include "mbuf.hpp"
#include <iostream>

using namespace std;

int main()
{
	m_buffer mb(3, 0x8);
	
	cout<<mb.is_avail()<<endl;

	mb.push("1234", 5);
	mb.push("4567", 5);
	mb.push("7890", 5);
	mb.push("abcd", 5);
	mb.push("efgh", 5);
	mb.push("ijkl", 5);
	mb.push("mnop", 5);
	
	mb.dump();

	char const* data;
	unsigned int size;
    while(mb.is_avail()){
        cout<<"send id: "<<mb.send(&data, &size)<<endl;
        cout<<"data: "<<data<<endl;
    }
    time_t recv_time = time(0);

	// normal ack
	cout<<"\nnormal ack: ack(1)\n";
	cout<<"Is aborted: "<<mb.is_aborted(1, recv_time)<<endl;
	if(!mb.is_aborted(1, recv_time)){
		cout<<"Do ack ..."<<endl;
		mb.ack(1);
	}
	mb.dump();
	
	cout<<"\nabnormal ack: ack(3)\n";
	cout<<"Is aborted: "<<mb.is_aborted(3, recv_time)<<endl;
	if(!mb.is_aborted(3, recv_time)){
		cout<<"Do ack ..."<<endl;
		mb.ack(3);
		// this ack will cause data 0 to be aborted
	}
	cout<<"Is data[0] aborted: "<<mb.is_aborted(1, recv_time)<<endl;
	mb.dump();

	cout<<"\nresend after abort:\n";
	while(mb.is_avail()){
		//if(!mb.isSent()){
			cout<<"send id: "<<mb.send(&data, &size)<<endl;
			cout<<"data: "<<data<<endl;
		//}
	}
	
	mb.dump();

	return 0;
}
