#pragma once
#include <boost/date_time/gregorian/gregorian.hpp>
#include <vector>
#include <bitset>

#include <boost/date_time/gregorian/greg_serialize.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/bitset.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/bimap.hpp>
#include <boost/mpl/map.hpp>


namespace mpl = boost::mpl;

namespace navitia { namespace type {
typedef unsigned int idx_t;
enum Type_e {eValidityPattern = 0,
    eLine = 1,
    eRoute = 2,
    eVehicleJourney = 3,
    eStopPoint = 4,
    eStopArea = 5,
    eStopTime = 6,
    eNetwork = 7,
    eMode = 8,
    eModeType = 9,
    eCity = 10,
    eConnection = 11,
    eRoutePoint = 12,
    eDistrict = 13,
    eDepartment = 14,
    eCompany = 15,
    eVehicle = 16,
    eCountry = 17,
    eUnknown = 18
};
struct PT_Data;
template<class T> std::string T::* name_getter(){return &T::name;}
template<class T> int T::* idx_getter(){return &T::idx;}


struct Nameable{
    std::string name;
    std::string comment;
};



struct NavitiaHeader{
    std::string id;
    idx_t idx;
    std::string external_code;
    NavitiaHeader() : idx(0){}
    std::vector<idx_t> get(Type_e, const PT_Data &) const {return std::vector<idx_t>();}

};

/**
 * Représente un systéme de projection
 * le constructeur par défaut doit renvoyer le sytéme de projection utilisé par l'application en interne
 * WGS84 dans notre cas
 */
struct Projection{
    std::string name;
    std::string definition;
    bool is_degree;

    Projection(): name("wgs84"), definition("+init=epsg:4326"), is_degree(true){}

    Projection(const std::string& name, const std::string& definition, bool is_degree = false):
        name(name), definition(definition), is_degree(is_degree){}

};

struct GeographicalCoord{
    double x;
    double y;

    GeographicalCoord() : x(0), y(0) {}
    GeographicalCoord(double x, double y) : x(x), y(y) {}
    GeographicalCoord(double x, double y, const Projection& projection);

    /* Calcule la distance Grand Arc entre deux nœuds
      *
      * On utilise la formule de Haversine
      * http://en.wikipedia.org/wiki/Law_of_haversines
      */
    double distance_to(const GeographicalCoord & other);

    template<class Archive> void serialize(Archive & ar, const unsigned int ) {
        ar & x & y;
    }

    GeographicalCoord convert_to(const Projection& projection, const Projection& current_projection = Projection()) const;
};

struct Country: public NavitiaHeader, Nameable {
    const static Type_e type = eCountry;
    idx_t main_city_idx;
    std::vector<idx_t> district_list;
    template<class Archive> void serialize(Archive & ar, const unsigned int ) {

        ar & name & main_city_idx & district_list & idx;
    }
    std::vector<idx_t> get(Type_e type, const PT_Data & data) const;
};

struct District : public NavitiaHeader, Nameable {
    const static Type_e type = eDistrict;
    idx_t main_city_idx;
    idx_t country_idx;
    std::vector<idx_t> department_list;
    template<class Archive> void serialize(Archive & ar, const unsigned int ) {
        ar & name & main_city_idx & country_idx & department_list & idx;
    }
};

struct Department : public NavitiaHeader, Nameable {
    const static Type_e type = eDepartment;
    idx_t main_city_idx;
    idx_t district_idx;
    std::vector<idx_t> city_list;
    template<class Archive> void serialize(Archive & ar, const unsigned int ) {
        ar & name & main_city_idx & district_idx & city_list & idx;
    }
};


struct City : public NavitiaHeader, Nameable {
    const static Type_e type = eCity;
    std::string main_postal_code;
    bool main_city;
    bool use_main_stop_area_property;

    idx_t department_idx;
    GeographicalCoord coord;

    std::vector<idx_t> postal_code_list;
    std::vector<idx_t> stop_area_list;
    std::vector<idx_t> address_list;
    std::vector<idx_t> site_list;
    std::vector<idx_t> stop_point_list;
    std::vector<idx_t> hang_list;
    std::vector<idx_t> odt_list;

    City() : main_city(false), use_main_stop_area_property(false), department_idx(0){}

    template<class Archive> void serialize(Archive & ar, const unsigned int ) {
        ar & name & department_idx & coord & idx;
    }

    std::vector<idx_t> get(Type_e type, const PT_Data & data) const;
};

struct Connection: public NavitiaHeader{
    const static Type_e type = eConnection;
    idx_t departure_stop_point_idx;
    idx_t destination_stop_point_idx;
    int duration;
    int max_duration;

    Connection() : departure_stop_point_idx(0), destination_stop_point_idx(0), duration(0),
        max_duration(0){};
    
    template<class Archive> void serialize(Archive & ar, const unsigned int ) {
        ar & id & idx & external_code & departure_stop_point_idx & destination_stop_point_idx & duration & max_duration;
    }
};

struct StopArea : public NavitiaHeader, Nameable{
    const static Type_e type = eStopArea;
    GeographicalCoord coord;
    int properties;
    std::string additional_data;
    idx_t city_idx;

    template<class Archive> void serialize(Archive & ar, const unsigned int ) {
        ar & id & idx & external_code & name & city_idx & coord;
    }

    StopArea(): properties(0), city_idx(0){}

    std::vector<idx_t> stop_point_list;
    std::vector<idx_t> get(Type_e type, const PT_Data & data) const;
};

struct Network : public NavitiaHeader, Nameable{
    const static Type_e type = eNetwork;
    std::string address_name;
    std::string address_number;
    std::string address_type_name;
    std::string phone_number;
    std::string mail;
    std::string website;
    std::string fax;

    std::vector<idx_t> line_list;

    template<class Archive> void serialize(Archive & ar, const unsigned int ) {
        ar & idx & id & name & external_code & address_name & address_number & address_type_name 
            & mail & website & fax & line_list;
    }

    std::vector<idx_t> get(Type_e type, const PT_Data & data) const;
};

struct Company : public NavitiaHeader, Nameable{
    const static Type_e type = eCompany;
    std::string address_name;
    std::string address_number;
    std::string address_type_name;
    std::string phone_number;
    std::string mail;
    std::string website;
    std::string fax;

    std::vector<idx_t> line_list;

    template<class Archive> void serialize(Archive & ar, const unsigned int ) {
        ar & idx & id & name & external_code & address_name & address_number & address_type_name & phone_number
                & mail & website & fax;
    }
    std::vector<idx_t> get(Type_e type, const PT_Data & data) const;
};

struct ModeType : public NavitiaHeader, Nameable{
    const static Type_e type = eModeType;
    std::vector<idx_t> mode_list;
    std::vector<idx_t> line_list;
    template<class Archive> void serialize(Archive & ar, const unsigned int ) {
        ar & idx & id & name & external_code & mode_list & line_list;
    }
    std::vector<idx_t> get(Type_e type, const PT_Data & data) const;
};

struct Mode : public NavitiaHeader, Nameable{
    const static Type_e type = eMode;
    idx_t mode_type_idx;
    template<class Archive> void serialize(Archive & ar, const unsigned int ) {
        ar & id & idx & name & external_code & mode_type_idx & idx;
    }
    std::vector<idx_t> get(Type_e type, const PT_Data & data) const;
};

struct Line : public NavitiaHeader, Nameable {
    const static Type_e type = eLine;
    std::string code;
    std::string forward_name;
    std::string backward_name;

    std::string additional_data;
    std::string color;
    int sort;

    idx_t mode_type_idx;

    std::vector<idx_t> mode_list;
    std::vector<idx_t> company_list;
    idx_t network_idx;

    std::vector<idx_t> forward_route;
    std::vector<idx_t> backward_route;

    std::vector<idx_t> impact_list;
    std::vector<idx_t> validity_pattern_list;

    idx_t forward_direction_idx;
    idx_t backward_direction_idx;

    Line(): sort(0), mode_type_idx(0), network_idx(0), forward_direction_idx(0), backward_direction_idx(0){}

    template<class Archive> void serialize(Archive & ar, const unsigned int ) {
        ar & id & idx & name & external_code & code & forward_name & backward_name & additional_data & color
                & sort & mode_type_idx & mode_list & company_list & network_idx & forward_direction_idx & backward_direction_idx
                & impact_list & validity_pattern_list;
    }
    std::vector<idx_t> get(Type_e type, const PT_Data & data) const;
};

struct Route : public NavitiaHeader, Nameable{
    const static Type_e type = eRoute;
    bool is_frequence;
    bool is_forward;
    bool is_adapted;
    idx_t line_idx;
    idx_t mode_type_idx;
    idx_t associated_route_idx;
    
    std::vector<idx_t> route_point_list;
    std::vector<idx_t> freq_route_point_list;
    std::vector<idx_t> freq_setting_list;
    std::vector<idx_t> vehicle_journey_list;
    std::vector<idx_t> impact_list;

    Route(): is_frequence(false), is_forward(false), is_adapted(false), line_idx(0), mode_type_idx(0), associated_route_idx(0){};

    template<class Archive> void serialize(Archive & ar, const unsigned int ) {
        ar & id & idx & name & external_code & is_frequence & is_forward & is_adapted & mode_type_idx
                & line_idx & associated_route_idx & route_point_list & freq_route_point_list & freq_setting_list
                & vehicle_journey_list & impact_list;
    }

    std::vector<idx_t> get(Type_e type, const PT_Data & data) const;
};

struct VehicleJourney: public NavitiaHeader, Nameable {
    const static Type_e type = eVehicleJourney;
    idx_t route_idx;
    idx_t company_idx;
    idx_t mode_idx;
    idx_t vehicle_idx;
    bool is_adapted;
    idx_t validity_pattern_idx;

    VehicleJourney(): route_idx(0), company_idx(0), mode_idx(0), vehicle_idx(0), is_adapted(false), validity_pattern_idx(0){}
    template<class Archive> void serialize(Archive & ar, const unsigned int ) {
        ar & name & external_code & route_idx & company_idx & mode_idx & vehicle_idx & is_adapted & validity_pattern_idx & idx;
    }
    std::vector<idx_t> get(Type_e type, const PT_Data & data) const;
};

struct Vehicle: public NavitiaHeader, Nameable {
    const static Type_e type = eVehicle;
};

struct Equipement : public NavitiaHeader {
    enum EquipementKind{ Sheltred, 
                         MIPAccess,
                         Escalator,
                         BikeAccepted,
                         BikeDepot,
                         VisualAnnouncement,
                         AudibleAnnoucement,
                         AppropriateEscort,
                         AppropriateSignage
                       };

    std::bitset<9> equipement_kind;
    
};

struct RoutePoint : public NavitiaHeader{
    const static Type_e type = eRoutePoint;
    int order;
    bool main_stop_point;
    int fare_section;
    idx_t route_idx;
    idx_t stop_point_idx;

    std::vector<idx_t> impact_list;

    RoutePoint() : order(0), main_stop_point(false), fare_section(0), route_idx(0), stop_point_idx(0){}

    template<class Archive> void serialize(Archive & ar, const unsigned int) {
        ar & id & idx & external_code & order & main_stop_point & fare_section & route_idx 
                & stop_point_idx & impact_list;
    }
    std::vector<idx_t> get(Type_e type, const PT_Data & data) const;
};

struct ValidityPattern : public NavitiaHeader {
    const static Type_e type = eValidityPattern;
private:
    boost::gregorian::date beginning_date;
    std::bitset<366> days;
    bool is_valid(int duration);
public:
    idx_t idx;
    ValidityPattern() : idx(0) {}
    ValidityPattern(boost::gregorian::date beginning_date) : beginning_date(beginning_date), idx(0){}
    void add(boost::gregorian::date day);
    void add(int day);
    void add(boost::gregorian::date start, boost::gregorian::date end, std::bitset<7> active_days);
    void remove(boost::gregorian::date day);
    void remove(int day);
    std::string str() const;
    template<class Archive> void serialize(Archive & ar, const unsigned int ) {
        ar & beginning_date & days & idx;
    }
};

struct StopPoint : public NavitiaHeader, Nameable{
    const static Type_e type = eStopPoint;
    GeographicalCoord coord;
    int fare_zone;

    std::string address_name;
    std::string address_number;
    std::string address_type_name;

    idx_t stop_area_idx;
    idx_t city_idx;
    idx_t mode_idx;
    idx_t network_idx;
    std::vector<idx_t> impact_list;
    std::vector<idx_t> route_point_list;
    template<class Archive> void serialize(Archive & ar, const unsigned int ) {
        ar & external_code & name & stop_area_idx & mode_idx & coord & fare_zone & idx & route_point_list;
    }

    StopPoint(): fare_zone(0),  stop_area_idx(0), city_idx(0), mode_idx(0), network_idx(0){}

    std::vector<idx_t> get(Type_e type, const PT_Data & data) const;
};

struct StopTime: public NavitiaHeader{
    const static Type_e type = eStopTime;
    int arrival_time; ///< En secondes depuis minuit
    int departure_time; ///< En secondes depuis minuit
    size_t vehicle_journey_idx;
    size_t route_point_idx;
    int order;
    bool ODT;
    int zone;


    StopTime(): arrival_time(0), departure_time(0), vehicle_journey_idx(0), route_point_idx(0), order(0), 
        ODT(false), zone(0){}

    template<class Archive> void serialize(Archive & ar, const unsigned int ) {
            ar & arrival_time & departure_time & vehicle_journey_idx & route_point_idx & order & ODT & zone 
                & idx & id & external_code;
    }
    std::vector<idx_t> get(Type_e type, const PT_Data & data) const;
};


struct static_data {
private:
    static static_data * instance;
public:
    static static_data * get();
    static std::string getListNameByType(Type_e type);
    static boost::posix_time::ptime parse_date_time(const std::string& s);
    static Type_e typeByCaption(const std::string & type_str);
    static std::string captionByType(Type_e type);
    boost::bimap<Type_e, std::string> types_string;
};

/** Correspondance entre les enums de type et les classes
  *
  *  L'utilisation des mpl::int_ est nécessaire car on ne peut pas passer d'énum de base (c'est naze)
  */
typedef boost::mpl::map< mpl::pair<mpl::int_<eValidityPattern>, ValidityPattern>,
                         mpl::pair<mpl::int_<eLine>, Line>,
                         mpl::pair<mpl::int_<eRoute>, Route>,
                         mpl::pair<mpl::int_<eVehicleJourney>, VehicleJourney>,
                         mpl::pair<mpl::int_<eStopPoint>, StopPoint>,
                         mpl::pair<mpl::int_<eStopArea>, StopArea>,
                         mpl::pair<mpl::int_<eStopTime>, StopTime>,
                         mpl::pair<mpl::int_<eNetwork>, Network>,
                         mpl::pair<mpl::int_<eMode>, Mode>,
                         mpl::pair<mpl::int_<eModeType>, ModeType>,
                         mpl::pair<mpl::int_<eCity>, City>,
                         mpl::pair<mpl::int_<eConnection>, Connection>,
                         mpl::pair<mpl::int_<eRoutePoint>, RoutePoint>,
                         mpl::pair<mpl::int_<eDistrict>, District>,
                         mpl::pair<mpl::int_<eDepartment>, Department>,
                         mpl::pair<mpl::int_<eCompany>, Company>,
                         mpl::pair<mpl::int_<eVehicle>, Vehicle>,
                         mpl::pair<mpl::int_<eCountry>, Country>
                      > enum_type_map;

/** Type pour gérer le polymorphisme en entrée de l'API
  *
  * Les objets on un identifiant universel de type stop_area:872124
  * Ces identifiants ne devraient pas être générés par le média
  * C'est toujours NAViTiA qui le génère pour être repris tel quel par le média
  */
struct EntryPoint {
    Type_e type;//< Le type de l'objet
    std::string external_code; //< Le code externe de l'objet

    /// Retourne une string unique à parti de n'importe quel objet
    template<class T>
    static std::string get_uri(T t){
        return static_data::get()->captionByType(T::type) + ":" + t.external_code;
    }

    /// Construit le type à partir d'une chaîne
    EntryPoint(const std::string & uri){
        size_t pos = uri.find(":");
        if(pos == std::string::npos)
            type = eUnknown;
        else {
            type = static_data::get()->typeByCaption(uri.substr(0,pos));
            external_code = uri.substr(pos+1);
        }
    }

};

} } //namespace navitia::type
