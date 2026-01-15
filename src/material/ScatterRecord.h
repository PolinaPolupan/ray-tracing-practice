//
// Created by polup on 07/01/2026.
//

#ifndef SCATTERRECORD_H
#define SCATTERRECORD_H
#include <memory>

#include "pdf/PDF.h"

class ScatterRecord {
public:
    mutable color attenuation;
    mutable std::shared_ptr<PDF> pdfPtr;
    mutable bool skipPdf;
    mutable ray skipPdfRay;
};



#endif //SCATTERRECORD_H
