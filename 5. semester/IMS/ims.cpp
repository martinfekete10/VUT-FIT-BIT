/**
 * IMS projekt 2020/2021
 * Zadanie: Téma č. 2: Epidemiologické modely - modely na makroúrovni
 * Autori: Martin Fekete <xfeket00@fit.vutbr.cz>
 *         Jakub Sekula <xsekul01@fit.vutbr.cz>
 * 
 * Hodnoty konštant alfa, beta,... sigma sú nastavené podľa kódu
 * na konci: https://www.nature.com/articles/s41591-020-0883-7
 */


#include <iostream>
#include <string>
#include <boost/array.hpp>
#include <boost/numeric/odeint.hpp>

using namespace std;
using namespace boost::numeric::odeint;

double agerisk = 1.00;

// Transmission rate due to contacts with UNDETECTED asymptomatic infected
double alfa = agerisk * 0.57;
// Transmission rate due to contacts with DETECTED asymptomatic infected
double beta1 = agerisk * 0.0114;
// Transmission rate due to contacts with UNDETECTED symptomatic infected
double gamma1 = agerisk * 0.456;
// Transmission rate due to contacts with DETECTED symptomatic infected
double delta = agerisk * 0.0114;

// Detection rate for ASYMPTOMATIC
double epsilon = 0.171;
// Detection rate for SYMPTOMATIC
double theta = 0.3705;

// Worsening rate: UNDETECTED asymptomatic infected becomes symptomatic
double zeta = 0.1254;
// Worsening rate: DETECTED asymptomatic infected becomes symptomatic
double eta = 0.1254;

// Worsening rate: UNDETECTED symptomatic infected develop life-threatening symptoms
double mu = 0.0171;
// Worsening rate: DETECTED symptomatic infected develop life-threatening symptoms
double nu = 0.0274;

// Mortality rate for infected with life-threatening symptoms
double tau = 0.01;

// Recovery rate for undetected asymptomatic infected
double lambda = 0.0342;
// Recovery rate for detected asymptomatic infected
double rho = 0.0342;
// Recovery rate for undetected symptomatic infected
double kappa = 0.0171;
// Recovery rate for detected symptomatic infected
double xi = 0.0171;
// Recovery rate for life-threatened symptomatic infected
double sigma = 0.0171;

double population = 60000000;
vector<int> restrictions;
vector<int> days;
int end_day;
int format;

// Array that will contain current info about pandemy
typedef boost::array <double, 10> state_type;

void print_help() {
    cout << "Usage: ./ims -d days_array -r restriction_array -e end_day -c cumulated\n";
    cout << "-d days_array : days, when new restrictions are introduced separated by ','\n";
    cout << "-r restriction_array : codes of newly introduced restrictions separated by ',', codes are explained in doc\n";
    cout << "-e end_day : end of simulation\n";
    cout << "-f format : constant specifying output format, explained doc\n";
    cout << "Example: ./ims -d 4,12,22,28,38,50 -r 1,2,3,4,5,7 -e 350 -c 1\n";
    exit(0);
}

// Converts arguments separated by ',' to vector
// E.g. arg string 10,20,30,50 is converted to vector <10 | 50 | 30 | 50>
void arg_to_vect(const string &str, vector<int> &result) {
    size_t prev = 0, pos = 0;

    while (pos < str.length() && prev < str.length()) {

        pos = str.find(',', prev);
        if (pos == string::npos) {
            pos = str.length();
        }

        string token = str.substr(prev, pos-prev);
        result.push_back(atoi(token.c_str()));
        prev = pos + 1;
    }
}


/**
 * Argument parse
 */
void get_args(int argc, char** argv){
    int arg;
    while ((arg = getopt(argc, argv, "d:r:e:f:h")) != -1) {
        switch(arg) {
            case 'd':
                arg_to_vect(optarg, days);
                break;
            case 'r':
                arg_to_vect(optarg, restrictions);
                break;
            case 'e':
                end_day = atoi(optarg);
                break;
            case 'f':
                format = atoi(optarg);
                break;
            case 'h':
                print_help();
            default:
                cerr << "Uknown argument\n";
                print_help();
        }
    }

    if (argc < 4) {
        print_help();
    }

    if (days.size() != restrictions.size()) {
        cerr << "Number of restrictions should be the same as number of days\n";
        exit(0);
    }
}


/**
 * Differential equations defining SIDARTHE model
 */
void sidarthe(const state_type &x , state_type &dxdt , double t) {
    dxdt[0] = -x[0] * (alfa * x[1] + beta1 * x[2] + gamma1 * x[3] + delta * x[4]);
    dxdt[1] = x[0] * (alfa * x[1] + beta1 * x[2] + gamma1 * x[3] + delta * x[4]) - (epsilon + zeta + lambda) * x[1];
    dxdt[2] = epsilon * x[1] - (eta + rho) * x[2];
    dxdt[3] = zeta * x[1] - (theta + mu + kappa) * x[3];
    dxdt[4] = eta * x[2] + theta * x[3] - (nu + xi) * x[4];
    dxdt[5] = mu * x[3] + nu * x[4] - (sigma + tau) * x[5];
    dxdt[6] = lambda * x[1] + rho * x[2] + kappa * x[3] + xi * x[4] + sigma * x[5];
    dxdt[7] = tau * x[5];
    dxdt[8] = rho * x[2]  + xi * x[4] + sigma * x[5];
    dxdt[9] = x[0] * (alfa * x[1] + beta1 * x[2] + gamma1 * x[3] + delta * x[4]);
}


/**
 * Writes current state of integration to stdout
 * If argument -c is set to nonzero value, cumulated numbers are written
 * Otherwise actual cases in given time are written
 */
void write_sidarthe(const state_type &x , const double t) {
    
    // Write cumulated numbers
    if (format == 1) {
        cout << t << ' ' << x[9] << ' ' << x[1] + x[2] + x[3] + x[4] + x[5] << ' ' << x[6] << ' ' << x[ 7 ] << ' ' << x[2] + x[4] + x[5] + x[7] + x[8] << ' ' << x[2] + x[4] + x[5] << ' ' << x[8] << endl;
    
    // Write actual cases
    } else if (format == 2) {
        cout << t << ' ' << x[1] << ' ' << x[2] << ' ' << x[3] << ' ' << x[ 4 ] << ' ' << x[ 5 ] << endl;
    
    // Deaths and cumulated cases
    } else if (format == 3) {
        cout << t << ' ' << x[7] * population << ' ' << x[9] * population << endl;

    // Deaths only
    } else {
        cout << t << ' ' << x[7] * population << endl;

    } 
}


// -----------------------------------
// ----------- RESTRICTIONS ----------

// No restrictions at all
// Days 0-4 in original simulation
void restriction0() {
    alfa = agerisk * 0.57;
    beta1 = agerisk * 0.0114;
    gamma1 = agerisk * 0.456;
    delta = agerisk * 0.0114;

    epsilon = 0.171;
    theta = 0.3705;

    zeta = 0.1254;
    eta = 0.1254;

    mu = 0.0171;
    nu = 0.0274;
    tau = 0.01;

    lambda = 0.0342;
    rho = 0.0342;
    kappa = 0.0171;
    xi = 0.0171;
    sigma = 0.0171;
}

// Introduction of basic social distancing, awareness of the epidemic,
// hygiene and behavioral recommendations, and early measures by gov
// Days 4-12 in original simulation
void restriction1() {
    restriction0();
    alfa = agerisk * 0.4218;
    beta1 = agerisk * 0.0057;
    gamma1 = agerisk * 0.285;
    delta = agerisk * 0.0057;
}

// Asymptomatic individuals were almost no longer detected,
// screening focused on symptomatic individuals
// Days 12-22 in original simulation
void restriction2() {
    restriction1();
    epsilon = 0.1425;
}

// Partially incomplete lockdown, of which the effectiveness was
// reduced by the movement of people from the north to the south of Italy
// when the country-wide lockdown was announced but not yet enforced
// Days 22-28 in original simulation
void restriction3() {
    restriction2();
    alfa = agerisk * 0.36;
    beta1 = agerisk * 0.005;
    gamma1 = agerisk * 0.2;
    delta = agerisk * 0.005;
    epsilon = 0.1425;

    mu = 0.008;
    nu = 0.015;

    zeta = 0.034;
    eta = 0.034;

    lambda = 0.08;
    rho = 0.0171;
    kappa = 0.0171;
    xi = 0.0171;
    sigma = 0.0171;
}

// National lockdown
// Days 28-38 in original simulation
void restriction4() {
    restriction3();
    alfa = agerisk * 0.21;
    gamma1 = agerisk * 0.11;
}

// Broader diagnosis campaign
// Days 38-50 in original simulation
void restriction5() {
    restriction4();
    epsilon = 0.2;
    rho = 0.02;
    kappa = 0.02;
    xi = 0.02;
    sigma = 0.01;
        
    zeta = 0.025;
    eta = 0.025;
}

// Strengthened lockdown
void restriction6() {
    restriction5();
    alfa = agerisk * 0.21*0.5;
    beta1 = agerisk * 0.005;
    gamma1 = agerisk * 0.11;
    delta = agerisk * 0.005;
    
    epsilon = 0.2;
    theta = 0.3705;
    
    zeta = 0.025;
    eta = 0.025;
    
    mu = 0.008;
    nu = 0.015;

    tau   = 0.01;

    lambda = 0.08;
    rho = 0.02;
    kappa = 0.02;
    xi = 0.02;
    sigma = 0.01;
}

// Weakened lockdown
void restriction7() {
    restriction5();
    alfa = agerisk * 0.21*1.2;
    beta1 = agerisk * 0.005;
    gamma1 = agerisk * 0.11;
    delta = agerisk * 0.005;
    
    epsilon = 0.2;
    theta = 0.3705;
    
    zeta = 0.025;
    eta = 0.025;
    
    mu = 0.008;
    nu = 0.015;

    tau = 0.01;

    lambda = 0.08;
    rho = 0.02;
    kappa = 0.02;
    xi = 0.02;
    sigma = 0.01;
}

// Widespread testing
void restriction8() {
    restriction5();
    alfa = 0.21;
    beta1 = 0.005;
    gamma1 = 0.11;
    delta = 0.005;
    
    epsilon = 0.2*2;
    theta = 0.3705;
    
    zeta = 0.0250;
    eta = 0.0250;
    
    mu = 0.008;
    nu = 0.0150;
    
    tau = 0.01;
    
    lambda = 0.08;
    rho = 0.02;
    kappa = 0.02;
    xi = 0.02;
    sigma = 0.01;
}

// Weakened lockdown with widespread testing
void restriction9() {
    restriction5();
    alfa = 0.21*2;
    beta1 = 0.005;
    gamma1 = 0.11;
    delta = 0.005;
    
    epsilon =  0.2*3;
    theta = 0.3705;
    
    zeta = 0.025;
    eta = 0.025;
    
    mu = 0.008;
    nu = 0.0150;
    
    tau = 0.01;
    
    lambda = 0.08;
    rho = 0.02;
    kappa = 0.02;
    xi = 0.02;
    sigma = 0.01;
}


/**
 * Sets restrictions given in argument
 * Strength of restrictions is specified in docu
 */
void set_restriction(int restriction) {
    if (restriction == 0) {
        restriction0();
    } else if (restriction == 1) {
        restriction1();
    } else if (restriction == 2) {
        restriction2();
    } else if (restriction == 3) {
        restriction3();
    } else if (restriction == 4) {
        restriction4();
    } else if (restriction == 5) {
        restriction5();
    } else if (restriction == 6) {
        restriction6();
    } else if (restriction == 7) {
        restriction7();
    } else if (restriction == 8) {
        restriction8();
    } else {
        restriction9();
    }
}

int main(int argc, char **argv) {
    
    get_args(argc, argv);
    double S0 = 1.0 - 200.0/population - 20.0/population - 1.0/population - 2.0/population - 0.0 - 0.0 - 0.0;
    
    // Initial state
    //             S[0]      I[1]              D[2]             A[3]           R[4]        T[5] H[6] E[7] x[8] x[9]
    state_type x = {S0, 200.0/population, 20.0/population, 1.0/population, 2.0/population, 0.0, 0.0, 0.0, 0.0, 0.0};

    double start_integrate = 0.0;
    double end_integrate = days.at(0);
    
    // 0 - day of first restriction
    set_restriction(0);
    integrate(sidarthe, x, 0.0, end_integrate, 0.01, write_sidarthe);
    
    // Setting restrictions and simulatiog
    for (int i = 0; i < (int) days.size(); i++) {
        
        if (i == (int) days.size() - 1) {
            start_integrate = days.at(i);
            end_integrate = end_day;
        } else {
            start_integrate = days.at(i);
            end_integrate = days.at(i + 1);
        }

        set_restriction(restrictions.at(i));
        integrate(sidarthe, x, start_integrate, end_integrate, 0.01, write_sidarthe);
    }

    return 0;
} 