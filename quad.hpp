#include <opencv2/opencv.hpp>
#include <fstream>
#include <map>
#include "color.hpp"

//! Class representing a quadtree.
/*!
	The class uses a nested class for node representation.
	*/
class QuadTree {
	private:
		//! Class for node representation.
		/*!
			Each node contains pointers to its childrens and its color. Then a pointer for the root node is stored in this class.
			*/
		class Node {
			private:
				Color color; /*!< Color variable. */
				static double diffThreshold; /*!< Threshold for the maximum difference of a region. */
				static std::map<char, Color> colorMap; /*!< Maps characters to the Color enum. */
				static std::map<char, Color> initializeColorMap(); /*!< Initializes the static coloMap. */
				//! Converts a cv::Scalar variable to a Color enum.
				/*!
					The input parameter is converted to the closest predefined color with the Euclidian metric.
					\param cv::Scalar value.
					\return Color enum.
					*/
				Color scalar2Color(cv::Scalar);
				//! Converts a Color variable to char.
				/*!
					The input parameter is converted to the corresponding char.
					\param Color value.
					\return char enum.
					*/
				char color2String(Color);
				//! Converts a Color variable to cv::Scalar.
				/*!
					\param Color enum.
					\return cv::Scalar value.
					*/
				cv::Scalar color2Scalar(Color);
			public:
				//! Composes image stored in node.
				/*!
					\param Width of region.
					\param Height of region.
					\param Boolean about drawing the boundaries of the node.
					\return cv::Mat object containing the image.
					*/
				cv::Mat compose(int, int, bool);
				//! Constructor
				/*!
					Initializes childs to nullptr.
					*/
				Node();
				//! Constructor with file.
				/*!
					\param Input file handle.
					*/
				Node(std::ifstream&);
				//! Constructor with image.
				/*!
					Decomposes the image if the maximum difference is above the given threshold, otherwise converts the average color to the Color enum.
					*/
				Node(cv::Mat);
				//! Destructor.
				~Node() = default;
				Node* nw, /*!< Northwest child. */
					* ne, /*!< Northeast child. */
					* sw, /*!< Southwest child. */
					* se; /*!< Southeast child. */
				//! Frees child pointers and self.
				void destroy();
				//! Print node to file.
				/*!
					The character | denotes the existence of child nodes and a single character denotes the predefined colors (w, b, r, g, k).
					\param Output file handle.
					*/
				void print(std::ofstream&);
				//! Set the color of the node.
				/*!
					\param character representing the color.
					*/
				void setColor(char);
		};
		Node* root; /*!< Pointer to root node. */
		int size_x, /*!< Width of full image. */
				size_y; /*!< Height of full image. */
	public:
		//! Constructor with filename.
		/*!
			Recursively parses quadtree starting from the root.
			\param Input filename.
			*/
		QuadTree(std::string);
		//! Constructor with image.
		/*!
			Recursively decomposes image building the quadtree.
			\param cv::Mat object containing the image.
			*/
		QuadTree(cv::Mat);
		//! Print quadtree to file.
		/*!
			Recursively prints nodes starting from the root into a .qd file (stands for quadtree). The file containd the width and height of the image, then the data.
			\param Output filename.
			*/
		void print(std::string);
		//! Build quadtree into image.
		/*!
			Recursively composes image starting from the root.
			\param Boolean about drawing the boundaries of the nodes.
			\return cv::Mat object containing the image.
			*/
		cv::Mat getImage(bool);
		//! Destructor.
		/*!
			Recursively frees nodes starting from the root.
			*/
		~QuadTree();
};
