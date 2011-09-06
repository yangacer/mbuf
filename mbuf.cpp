#include <cassert>
#include <ctime>
#include <deque>
#include "mbuf.hpp"

struct m_ele
{
	unsigned int id_;
	char *data_;
	unsigned int size_;
};

struct buf_handle
{
	buf_handle():send_cnt_(0){}

	unsigned int send_cnt_;
	std::deque<m_ele> dq_;	
};

m_buffer::m_buffer(unsigned int m_size, unsigned int max)
: 	pool_(m_size), seq_num_(0), snd_seq_(0), 
	ack_cnt_(0), abo_cnt_(0), max_(max), 
	last_reset_(std::time(0))
{
	assert( m_size != 0 );	
}

m_buffer::~m_buffer()
{
	using std::vector;
	using std::deque;

	for(unsigned int i = 0;i<pool_.size(); ++i){
		for(unsigned int j =0; j<pool_[i].dq_.size();++j){
			delete [](pool_[i].dq_[j].data_);
			pool_[i].dq_[j].data_ = 0;
		}
	}
}

void
m_buffer::dump()
{
	printf( "max: %d\t"
		"is_next_overflow: %d\t"
		"in_avail(): %d\t"
		"room: %d\n", 
		max_, is_next_overflow(), in_avail(), room());

	for(unsigned int i = 0;i<pool_.size(); ++i){
		printf(	"buffer[%u]\t"
			"send_cnt: %d\n", 
			i, pool_[i].send_cnt_);
		for(unsigned int j =0; j<pool_[i].dq_.size();++j){
			printf("\tid:%4d, data: %s\n", 
                pool_[i].dq_[j].id_, pool_[i].dq_[j].data_);
		}
	}
}

void
m_buffer::reset()
{
	// re-number all elements
	for(unsigned int i = 0;i<pool_.size(); ++i)
		for(unsigned int j =0; j<pool_[i].dq_.size();++j)
			pool_[i].dq_[j].id_ = i*pool_.size() + j;
	
	// update reset time
	last_reset_ = std::time(0);		
	
}

unsigned int
m_buffer::push(char const *data, unsigned int size)
{
	using std::deque;
	unsigned int pidx = seq_num_ % pool_.size();
	
	// allocate space for m_ele
	pool_[pidx].dq_.push_back(m_ele());
	
	deque<m_ele>::iterator last = (pool_[pidx].dq_.end() - 1);
	
	// assign sequence number and copy data
	last->id_ = seq_num_;
	last->data_ = new char[size];
	last->size_ = size;
	memcpy(last->data_, data, size);
	
	// increment sequence number
	seq_num_++;

	return seq_num_ - 1;
}

void
m_buffer::pop(unsigned int id)
{
	using std::deque;
	unsigned int pidx = id % pool_.size();

	deque<m_ele>::iterator first = pool_[pidx].dq_.begin();
	
	if(first == pool_[pidx].dq_.end())
		return;

	delete [](first->data_);
	first->data_ = 0;

	pool_[pidx].dq_.pop_front();
}


unsigned int
m_buffer::send(char const** output, unsigned int *size)
{
	unsigned int pidx = snd_seq_ % pool_.size();
	m_ele &ele = pool_[pidx].dq_[pool_[pidx].send_cnt_];

	*output = ele.data_;
	*size = ele.size_;
	
	pool_[pidx].send_cnt_++;
	snd_seq_++;

	return ele.id_;
}

bool
m_buffer::ack(unsigned int id)
{
	unsigned int pidx = id % pool_.size();
	if(id >= snd_seq_)
		return false;
	if(!is_front(id)){
		// abort
		unsigned int abort_cnt = 0;
		unsigned int npidx;
		// find acked one (linear search due to seq_num may overflowed
		while(id != pool_[pidx].dq_.front().id_){
			++abort_cnt;
			m_ele& aborted = pool_[pidx].dq_.front();
			// push to pool_ without allocating new data
			npidx = seq_num_ % pool_.size();
			aborted.id_ = seq_num_;
			pool_[npidx].dq_.push_back(aborted);
			seq_num_++;
			pool_[pidx].dq_.pop_front();
		}
		
		pool_[pidx].send_cnt_ -= abort_cnt;
		abo_cnt_ += abort_cnt;
		if(is_front(id)){
			pop(id);
			pool_[pidx].send_cnt_--;
			ack_cnt_ ++;
			return true;
		}
		return false;
	}

	pop(id);
	pool_[pidx].send_cnt_--;
	ack_cnt_++;
	return true;
}

unsigned int
m_buffer::in_avail()
{	return seq_num_ - ack_cnt_ - abo_cnt_;	}

unsigned int
m_buffer::room()
{	return max_ - seq_num_; }

bool
m_buffer::is_front(unsigned int id)
{
	unsigned int pidx = id % pool_.size();

	return (pool_[pidx].dq_.size() != 0 && 
		pool_[pidx].dq_.begin()->id_ == id);	
}

bool
m_buffer::is_avail()
{
	return seq_num_ > 0 && snd_seq_ < seq_num_;	
}

bool
m_buffer::is_aborted(unsigned int id, time_t stamp)
{
	return 	stamp < last_reset_ || 
		id < pool_[id % pool_.size()].dq_.front().id_;	
}

bool
m_buffer::is_next_overflow()
{ return (seq_num_+1) & ~max_; }


