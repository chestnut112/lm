#include "util.h"
#include <algorithm>
#include <fstream>
namespace arvin
{

    pid_t GetThreadId() { return syscall(SYS_gettid); }

    uint32_t GetFiberId()
    {
        return 0; 
    }

    std::string ToUpper(const std::string &name)
    {
        std::string rt = name;
        std::transform(rt.begin(), rt.end(), rt.begin(), ::toupper);
        return rt;
    }

    std::string ToLower(const std::string &name)
    {
        std::string rt = name;
        std::transform(rt.begin(), rt.end(), rt.begin(), ::tolower);
        return rt;
    }

    static int __lstat(const char *file, struct stat *st = nullptr)
    {
        struct stat lst;
        int ret = lstat(file, &lst);
        if (st)
        {
            *st = lst;
        }
        return ret;
    }

    static int __mkdir(const char *dirname)
    {
        if (access(dirname, F_OK) == 0)
        {
            return 0;
        }
        return mkdir(dirname, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    }
    bool FSUtil::Mkdir(const std::string &dirname)
    {
        if (__lstat(dirname.c_str()) == 0)
        {
            return true;
        }
        char *path = strdup(dirname.c_str());
        char *ptr = strchr(path + 1, '/');
        do
        {
            for (; ptr; *ptr = '/', ptr = strchr(ptr + 1, '/'))
            {
                *ptr = '\0';
                if (__mkdir(path) != 0)
                {
                    break;
                }
            }
            if (ptr != nullptr)
            {
                break;
            }
            else if (__mkdir(path) != 0)
            {
                break;
            }
            free(path);
            return true;
        } while (0);
        free(path);
        return false;
    }

    bool FSUtil::IsRunningPidfile(const std::string &pidfile)
    {
        if (__lstat(pidfile.c_str()) != 0)
        {
            return false;
        }
        std::ifstream ifs(pidfile);
        std::string line;
        if (!ifs || !std::getline(ifs, line))
        {
            return false;
        }
        if (line.empty())
        {
            return false;
        }
        pid_t pid = atoi(line.c_str());
        if (pid <= 1)
        {
            return false;
        }
        if (kill(pid, 0) != 0)
        {
            return false;
        }
        return true;
    }

    bool FSUtil::Unlink(const std::string &filename, bool exist)
    {
        if (!exist && __lstat(filename.c_str()))
        {
            return true;
        }
        return ::unlink(filename.c_str()) == 0;
    }

    bool FSUtil::Rm(const std::string &path)
    {
        struct stat st;
        if (lstat(path.c_str(), &st))
        {
            return true;
        }
        if (!(st.st_mode & S_IFDIR))
        {
            return Unlink(path);
        }

        DIR *dir = opendir(path.c_str());
        if (!dir)
        {
            return false;
        }

        bool ret = true;
        struct dirent *dp = nullptr;
        while ((dp = readdir(dir)))
        {
            if (!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, ".."))
            {
                continue;
            }
            std::string dirname = path + "/" + dp->d_name;
            ret = Rm(dirname);
        }
        closedir(dir);
        if (::rmdir(path.c_str()))
        {
            ret = false;
        }
        return ret;
    }

    bool FSUtil::Mv(const std::string &from, const std::string &to)
    {
        if (!Rm(to))
        {
            return false;
        }
        return rename(from.c_str(), to.c_str()) == 0;
    }

    bool FSUtil::Realpath(const std::string &path, std::string &rpath)
    {
        if (__lstat(path.c_str()))
        {
            return false;
        }
        char *ptr = ::realpath(path.c_str(), nullptr);
        if (nullptr == ptr)
        {
            return false;
        }
        std::string(ptr).swap(rpath);
        free(ptr);
        return true;
    }

    bool FSUtil::Symlink(const std::string &from, const std::string &to)
    {
        if (!Rm(to))
        {
            return false;
        }
        return ::symlink(from.c_str(), to.c_str()) == 0;
    }

    std::string FSUtil::Dirname(const std::string &filename)
    {
        if (filename.empty())
        {
            return ".";
        }
        auto pos = filename.rfind('/');
        if (pos == 0)
        {
            return "/";
        }
        else if (pos == std::string::npos)
        {
            return ".";
        }
        else
        {
            return filename.substr(0, pos);
        }
    }

    std::string FSUtil::Basename(const std::string &filename)
    {
        if (filename.empty())
        {
            return filename;
        }
        auto pos = filename.rfind('/');
        if (pos == std::string::npos)
        {
            return filename;
        }
        else
        {
            return filename.substr(pos + 1);
        }
    }

    bool FSUtil::OpenForRead(std::ifstream &ifs, const std::string &filename, std::ios_base::openmode mode)
    {
        ifs.open(filename.c_str(), mode);
        return ifs.is_open();
    }

    bool FSUtil::OpenForWrite(std::ofstream &ofs, const std::string &filename, std::ios_base::openmode mode)
    {
        ofs.open(filename.c_str(), mode);
        if (!ofs.is_open())
        {
            std::string dir = Dirname(filename);
            Mkdir(dir);
            ofs.open(filename.c_str(), mode);
        }
        return ofs.is_open();
    }
}