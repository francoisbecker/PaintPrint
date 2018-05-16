/**
  @file      main.cpp
  @copyright François Becker
  @date      2017-2018
  */

#include "pp_project.hpp"

#include <QCoreApplication>

#include <iostream>
#include <sstream>

/**
 * @todo integrate this Config class into the PP::Project class.
 */
struct Config
{
    std::string mExecPath;
    std::string mImagePath;
    std::string mOutputRootPath;
    float       mWidthMM;
    float       mPrintAreaXMM;
    float       mPrintAreaYMM;
    bool        mToolRefilling = false;
    float       mToolWidthMM = 1.f;
    std::string mToolColor;
    float       mToolDragErrorMM;
    std::string mToolRefillCommandFilePath;
    float       mLengthBeforeRefillMM = 300.f;
    int         mToolDryTimeSeconds = 20;
    std::vector<float> mLayersThresholds;

    Config(int argc, char* argv[])
    {
        parse(argc, argv);
    }

    void parse(int argc, char* argv[])
    {
        int i = 0;
        while (i < argc)
        {
            if (i == 0)
            {
                mExecPath = argv[i];
            }
            else if (std::string(argv[i]) == "-i")
            {
                if (i + 1 < argc)
                {
                    mImagePath = argv[++i];
                }
                else
                {
                    std::cerr << "-i expects an image path" << std::endl;
                    std::cerr << usage() << std::flush;
                    exit(EXIT_FAILURE);
                }
            }
            else if (std::string(argv[i]) == "-o")
            {
                if (i + 1 < argc)
                {
                    mOutputRootPath = argv[++i];
                }
                else
                {
                    std::cerr << "-o expects an output path" << std::endl;
                    std::cerr << usage() << std::flush;
                    exit(EXIT_FAILURE);
                }
            }
            else if (std::string(argv[i]) == "-ow")
            {
                if (i + 1 < argc)
                {
                    mWidthMM = std::atof(argv[++i]);
                }
                else
                {
                    std::cerr << "-ow expects an output image width in mm" << std::endl;
                    std::cerr << usage() << std::flush;
                    exit(EXIT_FAILURE);
                }
            }
            else if (std::string(argv[i]) == "-pa")
            {
                if (i + 2 < argc)
                {
                    mPrintAreaXMM = std::atof(argv[++i]);
                    mPrintAreaYMM = std::atof(argv[++i]);
                }
                else
                {
                    std::cerr << "-pa expects two values: x and y of the print area in mm." << std::endl;
                    std::cerr << usage() << std::flush;
                    exit(EXIT_FAILURE);
                }
            }
            else if (std::string(argv[i]) == "-tnr")
            {
                if (i + 4 < argc)
                {
                    mToolRefilling = false;
                    mToolWidthMM = std::atof(argv[++i]);
                    mToolColor = argv[++i];
                    mToolDragErrorMM = std::atof(argv[++i]);
                    mToolDryTimeSeconds = std::atoi(argv[++i]);
                }
                else
                {
                    std::cerr << "-tnr expects:\n"
                                 "      a width in mm,\n"
                                 "      a color,\n"
                                 "      a drag error in mm,\n"
                                 "      the dry time in seconds after a layer using this tool" << std::endl;
                    std::cerr << usage() << std::flush;
                    exit(EXIT_FAILURE);
                }
            }
            else if (std::string(argv[i]) == "-tr")
            {
                if (i + 6 < argc)
                {
                    mToolRefilling = true;
                    mToolWidthMM = std::atof(argv[++i]);
                    mToolColor = argv[++i];
                    mToolDragErrorMM = std::atof(argv[++i]);
                    mToolRefillCommandFilePath = argv[++i];
                    mLengthBeforeRefillMM = std::atof(argv[++i]);
                    mToolDryTimeSeconds = std::atoi(argv[++i]);
                }
                else
                {
                    std::cerr << "-tr expects:\n"
                                 "      a width in mm,\n"
                                 "      a color,\n"
                                 "      a drag error in mm,\n"
                                 "      a refill command file,\n"
                                 "      the length the paintbrush is able to draw with one refill in mm,\n"
                                 "      the dry time in seconds after a layer using this tool\n"
                              << std::endl;
                    std::cerr << usage() << std::flush;
                    exit(EXIT_FAILURE);
                }
            }
            else if (std::string(argv[i]) == "-l")
            {
                if (i + 1 < argc)
                {
                    float lThreshold = std::atof(argv[++i]);
                    mLayersThresholds.push_back(lThreshold);
                }
                else
                {
                    std::cerr << "-l expects a threshold value" << std::endl;
                    std::cerr << usage() << std::flush;
                    exit(EXIT_FAILURE);
                }
            }
            else
            {
                std::cerr << "Did not understand this argument: " << argv[i] << std::endl;
                std::cerr << usage() << std::flush;
                exit(EXIT_FAILURE);
            }
            ++i;
        }
    }

    static std::string usage()
    {
        return std::string("")
                + "Usage:\n"
                  "PaintPrint\n"
                  "      -i <image path>\n"
                  "      -o <output gcode file root (without .gcode)>\n"
                  "      -ow <output image width in mm>\n"
                  "      -pa <print area x in mm> <print area y in mm>\n"
                  "   tool selection and configuration:\n"
                  "      -tnr <width in mm> <color> <drag error in mm> <dry time in seconds> for a tool that does not need to refill\n"
                  " [or] -tr <width in mm> <color> <drag error in mm> <refill command file> <length before refill in mm> <dry time in seconds> for a tool that needs refilling\n"
                  "   passes/layers:\n"
                  "      -l <threshold> add a layer, this argument can be used multiple times\n";
    }

    bool isValid() const
    {
        return !mImagePath.empty()
                && !mOutputRootPath.empty()
                && mWidthMM != 0.f
                && mPrintAreaXMM != 0.f
                && mPrintAreaYMM != 0.f;
    }
};

int main(int argc, char *argv[])
{
    //QCoreApplication a(argc, argv);

    Config lConfig(argc, argv);

    if (!lConfig.isValid())
    {
        std::clog << "Invalid arguments list" << std::endl;
        std::clog << Config::usage() << std::flush;
        return EXIT_FAILURE;
    }

    PP::Project lProject;
    lProject.setImagePath(lConfig.mImagePath);
    lProject.setSaveRoot(lConfig.mOutputRootPath);
    lProject.setWidthMM(lConfig.mWidthMM);
    lProject.setPrintArea(lConfig.mPrintAreaXMM, lConfig.mPrintAreaYMM);
    QColor lColor(lConfig.mToolColor.c_str());
    if (!lConfig.mToolRefilling)
    {
        lProject.setTool(PP::Tool::noRefillTool("User tool",
                                                lConfig.mToolWidthMM,
                                                lColor,
                                                lConfig.mToolDragErrorMM,
                                                lConfig.mToolDryTimeSeconds));
    }
    else
    {
        std::ifstream lFile;
        lFile.open(lConfig.mToolRefillCommandFilePath);
        if (!lFile.is_open())
        {
            std::clog << "Could not load file " << lConfig.mToolRefillCommandFilePath << std::endl;
            return(EXIT_FAILURE);
        }
        std::string lRefillCommand { std::istreambuf_iterator<char>(lFile), std::istreambuf_iterator<char>() };
        lFile.close();
        PP::Tool lTool = PP::Tool::refillingTool("User refilling tool",
                                                 lConfig.mToolWidthMM,
                                                 lColor,
                                                 lConfig.mToolDragErrorMM,
                                                 lConfig.mLengthBeforeRefillMM,
                                                 lRefillCommand,
                                                 lConfig.mToolDryTimeSeconds);
        lProject.setTool(lTool);
    }
    for (float lThreshold : lConfig.mLayersThresholds)
    {
        lProject.addLayer(lThreshold);
    }
    std::cout << "Generating preview…" << std::endl;
    lProject.updatePreview();
    lProject.getPreview().save((lProject.getSaveRoot() + ".blended.jpg").c_str());
    for (int i = 0 ; i != lProject.getNumLayers() ; ++i)
    {
        QImage lLayerEssential = lProject.getLayerEssential(i).toImage();
        std::string lSavePath = (std::ostringstream() << lProject.getSaveRoot() << ".layer" << i << ".png").str();
        lLayerEssential.save(lSavePath.c_str());
    }
    std::cout << "Done." << std::endl;

    std::cout << "Generating project…" << std::endl;
    lProject.compileProject();
    std::cout << "Done." << std::endl;

    //return a.exec();
}
