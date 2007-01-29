class Geom::Point, Geom::Matrix, Geom::Scale, Geom::Rotate, Geom::Translate;
class NR::Point, NR::Matrix, NR::scale, NR::rotate, NR::translate;
struct NRMatrix;

Geom::Point     NR2Geom(NR::Point);
Geom::Matrix    NR2Geom(NR::Matrix);
Geom::Matrix    NR2Geom(NR::NRMatrix);
Geom::Scale     NR2Geom(NR::scale);
Geom::Rotate    NR2Geom(NR::rotate);
Geom::Translate NR2Geom(NR::translate);

NR::Point     Geom2NR(Geom::Point);
NR::Matrix    Geom2NR(Geom::Matrix);
NR::scale     Geom2NR(Geom::Scale);
NR::rotate    Geom2NR(Geom::Rotate);
NR::translate Geom2NR(Geom::Translate);
