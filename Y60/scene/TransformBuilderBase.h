#ifndef AC_SOM_TRANSFORM_BUILDER_BASE_INCLUDED
#define AC_SOM_TRANSFORM_BUILDER_BASE_INCLUDED

#include "WorldBuilderBase.h"

#include <asl/Vector234.h>
#include <asl/Quaternion.h>
#include <string>

namespace y60 {
    class TransformBuilderBase : public WorldBuilderBase {
        public:
            enum BillboardType { BB_NONE, BB_AXIS, BB_POINT };
            enum SensorType { SENSOR_NONE, SENSOR_APPROXIMATION };

            TransformBuilderBase(const std::string & theNodeName, const std::string & theName = "");
            virtual ~TransformBuilderBase();

            void setPosition(const asl::Vector3f & thePosition);
            void setOrientation(const asl::Vector3f & theOrientation);
            void setOrientation(const asl::Quaternionf & theOrientation);
            void setScale(const asl::Vector3f & theScale);
            void setPivot(const asl::Vector3f & thePivot);
            void setBillboard(const BillboardType theBillboardType);
            void setSensor(const SensorType theSensorType, float theSensorRadius);

        protected:
            TransformBuilderBase(const dom::NodePtr & theNode) :
                WorldBuilderBase(theNode)
            {}

        private:
            static unsigned _myNextId;
    };

}
#endif // AC_SOM_TRANSFORM_BUILDER_BASE_INCLUDED

