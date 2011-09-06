#ifndef _MBUF_H
#define _MBUF_H

#include <ctime>
#include <vector>

// forward decl
struct m_ele;
struct buf_handle;

struct m_buffer
{
public:
	m_buffer(unsigned int m_size, unsigned int max = 0xFFFF);
	~m_buffer();

	/** Push data into buffer
	 *  @param data
	 *  @param size Size of data
	 *  @return Numeric ID for the data.
	 */
	unsigned int
	push(char const *data, unsigned int size);

	/** Pop out data by id
	 *  @param id
	 */
	void
	pop(unsigned int id);

	/** Check wheather data with given id has been aborted
	 *  @param id
	 *  @return T/F
	 */
	bool
	is_aborted(unsigned int id, time_t stamp);

	/** Mark a data as sent
	 *  @param output pointer for accessing data
	 *  @param size pointer to varible gave by client for save size of data
	 *  @return Numeric ID for the data that propose to be used by followed
	 *  ack() and other checking such as isAborted(), etc.
	 */
	unsigned int
	send(char const** output, unsigned int *size);


	/** Acknowledge a data has been received
	 *  @param id
	 *  @return True when id exists.
	 */
	bool
	ack(unsigned int id);
	
	/** Count how many batches are available for sending
	 */
	unsigned int
	in_avail();

	/** Calculate how far from being overflowed
	 */
	unsigned int
	room();

	/** Re-numbering all data in this buffer and 
	 *  update last reset time
	 */
	void
	reset();

	/** Check wheather any data is unsent
	 */
	bool
	is_avail();
	
	/** Check wheather the sequence number used
	 *  to number data exceeds max value.
	 */
	bool
	is_next_overflow();
	
	void
	dump();

private:
	// disable copy
	m_buffer(m_buffer const &cp);
	m_buffer &operator=(m_buffer const &cp);

	bool
	is_front(unsigned int id);
	
	std::vector<buf_handle> pool_;
	unsigned int seq_num_;
	unsigned int snd_seq_;
	unsigned int ack_cnt_;
	unsigned int abo_cnt_;
	unsigned int max_;
	time_t last_reset_;
};


#endif
