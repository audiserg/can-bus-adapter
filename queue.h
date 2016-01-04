#ifndef QUEUE_H_
#define QUEUE_H_

#include <stdint.h>

template <int SIZE, typename EntryType>
class Queue {
    private:
        EntryType buffer[SIZE];
        unsigned int in;
        unsigned int out;
    
    public:
        Queue() {
            in = 0;
            out = 0;
        }
    
        void write(EntryType value) {
            buffer[in++] = value;
            in %= SIZE;
        }
    
        EntryType read() {
            EntryType result = buffer[out++];
            out %= SIZE;
            return result;
        }
    
        bool isEmpty() {
            return count() == 0;
        }
    
        bool isFull() {
            return count() == SIZE;
        }
        
        size_t count() {
            return in - out + (in >= out ? 0 : SIZE);
        }
};

#endif /* QUEUE_H_ */
