


# The Art of Writing Efficient Programs

<a href="https://www.packtpub.com/product/the-art-of-writing-efficient-programs/9781800208117?utm_source=github&utm_medium=repository&utm_campaign=9781800208117"><img src="https://static.packt-cdn.com/products/9781800208117/cover/smaller" alt="The Art of Writing Efficient Programs" height="256px" align="right"></a>

This is the code repository for [The Art of Writing Efficient Programs](https://www.packtpub.com/product/the-art-of-writing-efficient-programs/9781800208117?utm_source=github&utm_medium=repository&utm_campaign=9781800208117), published by Packt.

**An advanced programmer's guide to efficient hardware utilization and compiler optimizations using C++ examples**

## What is this book about?
The great free lunch of "performance taking care of itself" is over. Until recently, programs got faster by themselves as CPUs were upgraded, but that doesn't happen anymore. The clock frequency of new processors has almost peaked. New architectures provide small improvements to existing programs, but this only helps slightly. Processors do get larger and powerful, but most of this new power is consumed by the increased number of processing cores and other “extra” computing units. To write efficient software, you now have to know how to program by making good use of the available computing resources, and this book will teach you how to do that. 

This book covers the following exciting features:
* Discover how to use the hardware computing resources in your programs effectively
* Understand the relationship between memory order and memory barriers
* Familiarize yourself with the performance implications of different data structures and organizations
* Assess the performance impact of concurrent memory accessed and how to minimize it
* Discover when to use and when not to use lock-free programming techniques
* Explore different ways to improve the effectiveness of compiler optimizations
* Design APIs for concurrent data structures and high-performance data structures to avoid inefficiencies

If you feel this book is for you, get your [copy](https://www.amazon.com/dp/1800208111) today!

<a href="https://www.packtpub.com/?utm_source=github&utm_medium=banner&utm_campaign=GitHubBanner"><img src="https://raw.githubusercontent.com/PacktPublishing/GitHub/master/GitHub.png" 
alt="https://www.packtpub.com/" border="5" /></a>

## Instructions and Navigations
All of the code is organized into folders. For example, Chapter02.

The code will look like the following:
```
std::vector<double> v;
… add data to v …
std::for_each(v.begin(), v.end(),[](double& x){ ++x; });
```

**Following is what you need for this book:**
This book is for experienced developers and programmers who work on performance-critical projects and want to learn different techniques to improve the performance of their code. Programmers who belong to algorithmic trading, gaming, bioinformatics, computational genomics, or computational fluid dynamics communities can learn various techniques from this book and apply them in their domain of work.
Although this book uses the C++ language, the concepts demonstrated in the book can be easily transferred or applied to other compiled languages such as C, Java, Rust, Go, and more.

With the following software and hardware list you can run all code files present in the book (Chapter 1-12). For more instructions on how to run the code, click [here](https://github.com/PacktPublishing/The-Art-of-Writing-Efficient-Programs/tree/master/Chapter02#readme).
### Software and Hardware List
| Chapter | Software required | OS required |
| -------- | ------------------------------------ | ----------------------------------- |
| 1-12 | C++ compiler (GCC, Clang, Visual Studio, etc.) | Windows, Mac OS X, and Linux (Any) |
| 1-12 | Profiler (VTune, Perf, GoogleProf, etc.) | Windows, Mac OS X, and Linux (Any) |
| 1-12 | Benchmark Library (Google Bench) | Windows, Mac OS X, and Linux (Any) |

We also provide a PDF file that has color images of the screenshots/diagrams used in this book. [Click here to download it](https://static.packt-cdn.com/downloads/9781800208117_ColorImages.pdf).

### Errata

* Page 25: The sentence "for example, if the actual string is generated from a simulation that takes ten hours, the one hundred _seconds_ it takes to sort it is hardly worth noticing." must be read as "for example, if the actual string is generated from a simulation that takes ten hours, the one hundred _milliseconds_ it takes to sort it is hardly worth noticing."
* Page 45: The file name for the first code snippet must be read as 01a_compare_timer.C and the second code snippet must be read as 01a_compare_timer_a.C/01a_compare_timer_b.C.
* Page 116: The sentence "As we have seen, under the right circumstances, the CPU can do several operations per second" must be read as "As we have seen, under the right circumstances, the CPU can do several operations per cycle".

### Related products
* C++ High Performance - Second Edition [[Packt]](https://www.packtpub.com/product/c-high-performance-second-edition/9781839216541?utm_source=github&utm_medium=repository&utm_campaign=9781839216541) [[Amazon]](https://www.amazon.com/dp/1839216549)

* Software Architecture with C++ [[Packt]](https://www.packtpub.com/product/software-architecture-with-c/9781838554590?utm_source=github&utm_medium=repository&utm_campaign=9781838554590) [[Amazon]](https://www.amazon.com/dp/1838554599)

## Get to Know the Author
**Fedor G. Pikus**
is a chief engineering scientist in the Mentor IC Segment of Siemens Digital Industries Software and is responsible for the long-term technical direction of Calibre products, the design and architecture of software, and research into new software technologies. His previous roles included senior software engineer at Google and chief software architect at Mentor Graphics. Fedor is a recognized expert in high-performance computing and C++. He has presented his works at CPPCon, SD West, DesignCon, and in software development journals, and is also an O'Reilly author. Fedor has over 25 patents and over 100 papers and conference presentations on physics, EDA, software design, and C++.
### Download a free PDF

 <i>If you have already purchased a print or Kindle version of this book, you can get a DRM-free PDF version at no cost.<br>Simply click on the link to claim your free PDF.</i>
<p align="center"> <a href="https://packt.link/free-ebook/9781800208117">https://packt.link/free-ebook/9781800208117 </a> </p>
