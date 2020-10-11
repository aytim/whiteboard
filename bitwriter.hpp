#include <fstream>
#include <vector>

//! Class for writin a file bit-by-bit.
/*!	The class provides an interface for writing a file bit-by-bit. It uses a buffer of given size and accepts strings containing binary data (i.e., one and zero characters). */
class BitWriter {
	private:
		std::vector<char> buffer; /*!< Character buffer. */
		std::ofstream out; /*!< Output file. */
		size_t size; /*!< Buffer size. */
		size_t counter; /*!< Counts the written bits. */
		//! A private function to set the bit on the given index.
		/*!
			\param Index of bit.
			\param Value of bit.
			*/
		void setBit(int, int);
		//! A private function to handle buffering and flushes.
		/*!
			\param Value of bit to set.
			*/
		void writeBit(int);
	public:
		//! Constructor with filename.
		BitWriter(std::string);
		//! Destructor.
		~BitWriter();
		//! A private function to flush.
		void flush(); 
		//! Write binary string to output file.
		/*!
			\param String to be written.
			*/
		void write(std::string);
};
