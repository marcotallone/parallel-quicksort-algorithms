<!-- Improved compatibility of back to top link: See: https://github.com/othneildrew/Best-README-Template/pull/73 -->
<a name="readme-top"></a>

<!-- PROJECT SHIELDS -->
<!--
*** I'm using markdown "reference style" links for readability.
*** Reference links are enclosed in brackets [ ] instead of parentheses ( ).
*** See the bottom of this document for the declaration of the reference variables
*** for contributors-url, forks-url, etc. This is an optional, concise syntax you may use.
*** https://www.markdownguide.org/basic-syntax/#reference-style-links
-->
[![Forks][forks-shield]][forks-url]
[![Stargazers][stars-shield]][stars-url]
[![Issues][issues-shield]][issues-url]
[![MIT License][license-shield]][license-url]
[![LinkedIn][linkedin-shield]][linkedin-url]

<!-- PROJECT LOGO -->
<br />
<div align="center">
  <!-- <a href="https://github.com/marcotallone/latex">
    <img src="images/colilogo.png" alt="Logo" width="80" height="80">
  </a> -->

<h3 align="center">Parallel Quicksort Algorithms in MPI and OpenMP</h3>

  <p align="center">
    A collection of parallel quicksort algorithms implemented in MPI and OpenMP
    <br />
    <a href="https://github.com/marcotallone/parallel-quicksort-algorithms"><strong>Explore the docs »</strong></a>
    <br>
    <a href="./parallel-quicksort-marco-tallone.pdf"><strong>Read the official report »</strong></a>
    <br />
    <br />
    <a href="./apps/mpi_example.c">View Demo</a>
    ·
    <a href="https://github.com/marcotallone/parallel-quicksort-algorithms/issues">Report Bug</a>
    ·
    <a href="https://github.com/marcotallone/parallel-quicksort-algorithms/issues">Request Feature</a>
  </p>
</div>

<!-- TABLE OF CONTENTS -->
<details>
  <summary>Table of Contents</summary>
  <ol>
    <li>
      <a href="#about-the-project">About The Project</a>
      <ul>
        <li><a href="#project-structure">Project Structure</a></li>
        <li><a href="#built-with">Built With</a></li>
        <li><a href="#implemented-quicksort-functions">Implemented Quicksort Functions</a></li>
      </ul>
    </li>
    <li>
      <a href="#getting-started">Getting Started</a>
      <ul>
        <li><a href="#prerequisites">Prerequisites</a></li>
        <li><a href="#installation">Installation</a></li>
      </ul>
    </li>
    <li><a href="#usage">Usage</a></li>
    <li><a href="#license">License</a></li>
    <li><a href="#contact">Contact</a></li>
    <li><a href="#references">References</a></li>
    <li><a href="#acknowledgments">Acknowledgments</a></li>
  </ol>
</details>

<!-- ABOUT THE PROJECT -->
## About The Project

<!-- [![Product Name Screen Shot][product-screenshot]](https://example.com) -->

This repository contains a different **parallel** implementations of the well-known **quicksort** sorting **algorithms**.\
Multiple algorithms have been implemented in `C` both in distributed memory, using the `MPI` library, and in shared memory, with `OpenMP` directives. Among the studied algorithms this repository includes the following parallel implementations:

* **(Simple) Parallel Quicksort**

* **Hyperquicksort**

* **Parallel Sort by Regular Sampling (PSRS)**

* **Task Quicksort** (`OpenMP` only)

More implementation-specific details are explained below.\
The whole project is organized as explained in the next section. All of the functions for the different algorithms have been implemented in dedicated files in the `src/` folder. The compilation of the library is managed through the `CMake` build system as explained below. The `apps/` folder contains some ready-to-use scripts that show how to use the implemented functions.\
The `python/` folder contains some scripts for plotting and analyzing the scaling data obtained from the execution of the algorithms on the **ORFEO** cluster at AREA Science Park, Basovizza (TS). The data from the scaling analysis are available in the `datasets/` folder.\
Further details about the study and the results obtained with these implementations can be fount in the [dedicated report](./parallel-quicksort-marco-tallone.pdf) that can be found in this repository.

### Project Structure

The project is organized with the following structure:

```bash
.
├── 📂 apps # Ready-to-use scripts
│   └── main.c
├── 📂 datasets # Datasets with scaling data
├── 📂 include # Include headers
│   └── qsort.h
├── 🐍 python # Python scripts for plotting and analysis
│   ├── utils
│   └── scaling.ipynb
├── 📄 report.pdf # Report of the project
├── 📜 README.md # This file
├── ⚙ install.sh # Quick compilation and installation
├── mpi.job # MPI job script for Slurm
├── omp.job # OpenMP job script for Slurm
└── 📂 src # Source files
    └── qsort.c
```

### Built With

![C](https://img.shields.io/badge/C-00CCFF?style=for-the-badge&logo=c&logoColor=white)
![Cmake](https://img.shields.io/badge/Cmake-00A3FF?style=for-the-badge&logo=cmake&logoColor=white)
![MPI](https://img.shields.io/badge/MPI-007AFF?style=for-the-badge&logo=mpi&logoColor=white)
![OpenMP](https://img.shields.io/badge/OpenMP-0052FF?style=for-the-badge&logo=openmp&logoColor=white)
![Python](https://img.shields.io/badge/Python-0000FF?style=for-the-badge&logo=python&logoColor=white)
![NeoVim](https://img.shields.io/badge/NeoVim-0700C4?style=for-the-badge&logo=neovim&logoColor=white)

<p align="right">(<a href="#readme-top">back to top</a>)</p>

### Implemented Quicksort Functions

This file presents here the main characteristics of the implemented functions and explains their practical usage.
The main implemented sorting functions are the folowing:

* `void serial_qsort(data_t *, int, int, compare_t)`: serial version of the quicksort algorithm. Takes in input the array to be sorted, the starting and ending index of the array and a comparison function to be used for sorting.

* `void omp_task_qsort(data_t *, int, int, compare_t)`: task-based parallel version using OpenMP. Takes in input the array to be sorted, the starting and ending index of the array and a comparison function to be used for sorting. This function requires to be used inside a parallel region as follows:

    ```c
    #pragma omp parallel
    {
      #pragma omp single
      {
        omp_task_qsort(array, 0, N, compare_f);
      }
    }
    ```

* `void omp_parallel_qsort(data_t *, int, int, compare_t, int)`: shared memory version of the quicksort algorithm using OpenMP. Takes in input the array to be sorted, the starting and ending index of the array, a comparison function to be used for sorting and the depth of the recursive call (First call 0). Can be normally used as any other function in the main script.

* `void omp_hyperquicksort(data_t *, int, int, compare_t, int)`: shared memory version of the hyperquicksort algorithm using OpenMP. Takes in input the array to be sorted, the starting and ending index of the array, a comparison function to be used for sorting and the depth of the recursive call (First call is 0).

* `void omp_psrs(data_t *, int, int, compare_t)`: shared memory version of the PSRS algorithm using OpenMP. Takes in input the array to be sorted, the starting and ending index of the array and a comparison function to be used for sorting.

* `void MPI_Parallel_qsort(data_t **, int *, int *, int, MPI_Comm, MPI_Datatype, compare_t)`: distributed memory version of the quicksort algorithm using MPI. This function must be called after `MPI_Initialize()` to enable communication between multiple processes. Takes in input the local array to be sorted, the size of the local array, the rank of the process, the number of processes, the MPI communicator, the MPI specific datatype of the array elements and a comparison function to be used for sorting.

* `void MPI_Hyperquicksort(data_t **, int *, int *, int, MPI_Comm, MPI_Datatype, compare_t)`: distributed memory version of the hyperquicksort algorithm using MPI. This function must be called after `MPI_Initialize()` to enable communication between multiple processes. Takes in input the local array to be sorted, the size of the local array, the rank of the process, the number of processes, the MPI communicator, the MPI specific datatype of the array elements and a comparison function to be used for sorting.

* `void MPI_PSRS(data_t **, int *, int, int, int, MPI_Datatype, compare_t)`: distributed memory version of the PSRS algorithm using MPI. This function must be called after `MPI_Initialize()` to enable communication between multiple processes. Takes in input the local array to be sorted, the size of the local array, the size of the global array, the rank of the process, the number of processes, the MPI communicator, the MPI specific datatype of the array elements and a comparison function to be used for sorting.

The serial and shared memory versions sort the input array in place directly without the need of any additional operation. The MPI versions require a the master process to initially split the input array in multiple chunks and to actually send the chunks to the different processes. The chunks are then sorted in place by the single processes. These can be merged by the master process at the end of the function execution to check for sorting correctness.
The `mpi_example.c` and `omp_example.c` script in the `apps/` folder show some [usage examples](./apps/).

<p align="right">(<a href="#readme-top">back to top</a>)</p>

<!-- GETTING STARTED -->
## Getting Started

If you want to use the implemented `epyc` module you can follow these steps. It is anyway recommended to take a look at the scripts in the `apps/` folder for eventual [usage examples](./apps/example.py).

### Prerequisites

THe following are needed to compile and run the paralle versions of the implemented algorithms:

* `CMake` build system
* `OpenMP`
* `MPI` library
* Access to **ORFEO** or another cluster with `Slurm` job scheduler for scaling jobs

### Installation

Compilation of the implemented library is performed with the `cmake` system.\
By executing the following commands (and eventually specifying the correct `-DCMAKE_PREFIX_PATH` for eventual third party libraries if not found) all the `C` libraries will be compiled in the `build/` folder.

```bash
cmake -S . -B build/

make -C build/ -j<N> -D<COMPILATION_MODE>=ON
```

The library can be compiled in different modes. These includes the following:

* **serial** compilation: this will only compile the serial version of the library without any parallel algorithm.

* **openmp** compilation: this will compile the library with the OpenMP parallel algorithms, in order to enable this version compile with the option `-DOMP=ON`.

* **mpi** compilation: this will compile the complete library with the MPI parallel algorithms and also using OpenMP since these algorithm require it, in order to enable this version compile with the option `-DMPI=ON`.

All of these modes also include a debugging mode where no optimization is performed at compile time and some debugging flags are enabled. This can be enabled by adding the `-DDEBUG=ON` flag to the compilation command.

> [!NOTE]
> For a quick installation following these commands a `build.sh` script has been provided in the root folder. To compile more easily with this script you can pass the compile version you want as a command like argument. Accepted argument include `omp`, `mpi` or their debug versions `debug omp` and `debug mpi`.

After compilation the executables can be found in the `build/bin/` folder. In order to add a personal executable to be compiled with the library you just need to update the `MAIN_FILES` list in the provided `CMakeLists.txt` file by directly editing the file.\
In the root directory of the project there are also 2 Slurm job to showcase how to compile and use the library on the ORFEO cluster. These will run a scaling test to asses strong and weak scalability of the implementations depending on the specified input. The results of the test will be appended on a relative `csv` file in the `datasets/` folder. It's possible to visualize the file with an editor of choice or by running the `display.c` program if compiled by the cmake system. Further details on the scaling tests can be found in the `mpi_scaling.c` and `omp_scaling.c` files.

<p align="right">(<a href="#readme-top">back to top</a>)</p>

## Usage

To be able to use the implemented library in a `C` script it's possible to import the `qsort.h` header file and link to the library at compilation time. Compilation details are explained below.

```c
#include "qsort.h"
```

In the `apps/` folder the `mpi_example.c` and `omp_example.c` files provide a starting point with some basic [usage example](./apps/) of the implemented features.

<p align="right">(<a href="#readme-top">back to top</a>)</p>

<!-- LICENSE -->
## License

Distributed under the MIT License. See `LICENSE.txt` for more information.

<p align="right">(<a href="#readme-top">back to top</a>)</p>

<!-- CONTACT -->
## Contact

| Contact Me | |
| --- | --- |
| Mail | <marcotallone85@gmail.com> |
| LinkedIn | [LinkedIn Page](https://linkedin.com/in/marco-tallone-40312425b) |
| GitHub | [marcotallone](https://github.com/marcotallone) |

<p align="right">(<a href="#readme-top">back to top</a>)</p>

<!-- REFERENCES -->

## References

- [MPI Forum](https://www.mpi-forum.org/)
- [OpenMP](https://www.openmp.org/)
- [CMake](https://cmake.org/)
- [AREA Science Park](https://www.area.trieste.it/it/infrastrutture/orfeo)
- [ORFEO Documentation](https://orfeo-doc.areasciencepark.it/)
- [HPC UniTS](https://github.com/Foundations-of-HPC/High-Performance-Computing-2023)

<p align="right">(<a href="#readme-top">back to top</a>)</p>

<!-- ACKNOWLEDGMENTS -->
## Acknowledgments

* [Best-README-Template](https://github.com/othneildrew/Best-README-Template?tab=readme-ov-file)

<p align="right">(<a href="#readme-top">back to top</a>)</p>

<!-- MARKDOWN LINKS & IMAGES -->
<!-- https://www.markdownguide.org/basic-syntax/#reference-style-links -->
[contributors-shield]: https://img.shields.io/github/contributors/marcotallone/parallel-quicksort-algorithms.svg?style=for-the-badge
[contributors-url]: https://github.com/marcotallone/parallel-quicksort-algorithms/graphs/contributors
[forks-shield]: https://img.shields.io/github/forks/marcotallone/parallel-quicksort-algorithms.svg?style=for-the-badge
[forks-url]: https://github.com/marcotallone/parallel-quicksort-algorithms/network/members
[stars-shield]: https://img.shields.io/github/stars/marcotallone/parallel-quicksort-algorithms.svg?style=for-the-badge
[stars-url]: https://github.com/marcotallone/parallel-quicksort-algorithms/stargazers
[issues-shield]: https://img.shields.io/github/issues/marcotallone/parallel-quicksort-algorithms.svg?style=for-the-badge
[issues-url]: https://github.com/marcotallone/parallel-quicksort-algorithms/issues
[license-shield]: https://img.shields.io/github/license/marcotallone/parallel-quicksort-algorithms.svg?style=for-the-badge
[license-url]: https://github.com/marcotallone/parallel-quicksort-algorithms/blob/master/LICENSE
[linkedin-shield]: https://img.shields.io/badge/-LinkedIn-black.svg?style=for-the-badge&logo=linkedin&colorB=555
[linkedin-url]: https://linkedin.com/in/marco-tallone-40312425b
[product-screenshot]: images/screenshot.png
[Next.js]: https://img.shields.io/badge/next.js-000000?style=for-the-badge&logo=nextdotjs&logoColor=white
[Next-url]: https://nextjs.org/
[React.js]: https://img.shields.io/badge/React-20232A?style=for-the-badge&logo=react&logoColor=61DAFB
[React-url]: https://reactjs.org/
[Vue.js]: https://img.shields.io/badge/Vue.js-35495E?style=for-the-badge&logo=vuedotjs&logoColor=4FC08D
[Vue-url]: https://vuejs.org/
[Angular.io]: https://img.shields.io/badge/Angular-DD0031?style=for-the-badge&logo=angular&logoColor=white
[Angular-url]: https://angular.io/
[Svelte.dev]: https://img.shields.io/badge/Svelte-4A4A55?style=for-the-badge&logo=svelte&logoColor=FF3E00
[Svelte-url]: https://svelte.dev/
[Laravel.com]: https://img.shields.io/badge/Laravel-FF2D20?style=for-the-badge&logo=laravel&logoColor=white
[Laravel-url]: https://laravel.com
[Bootstrap.com]: https://img.shields.io/badge/Bootstrap-563D7C?style=for-the-badge&logo=bootstrap&logoColor=white
[Bootstrap-url]: https://getbootstrap.com
[JQuery.com]: https://img.shields.io/badge/jQuery-0769AD?style=for-the-badge&logo=jquery&logoColor=white
[JQuery-url]: https://jquery.com 
