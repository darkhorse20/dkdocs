#include "Minimizer.h"
#include "nr3.h"
#include "mins_ndim.h"
#include <random>

struct Func {
	
	Func(Filter *filter_type, double *ln_stock_prxs, double *res_estimates, double* u_vals, double *v_vals, int nr_prxs, double p_val): 
prxs(ln_stock_prxs), n(nr_prxs), p(p_val), estimates(res_estimates) , u(u_vals), v(v_vals), filter(filter_type)
		{	}

	Doub operator()(VecDoub_I & x) {
		
		//double *u = new double[n];
		//double *v = new double[n];
		//double *estimates = new double[n];

		//filter.estimate_extended_kalman_parameters_1_dim_fin_proj(prxs, 0.0, n, p,
		//	x[0],x[1],x[2],x[3], u, v, estimates);

		filter->applyFilter(prxs, 0.0, n, p,
			x[0],x[1],x[2],x[3], u, v, estimates);

		//estimate_extended_kalman_parameters_1_dim(prxs, 0.0, n,
		//	x[0],x[1],x[2],x[3], u, v, estimates);
		
		Doub neg_lli_hood = 0.0;
		for(int i=0; i < n; i++) {
			neg_lli_hood += log(v[i])+u[i]*u[i]/v[i];
		}

		//cout << "Log likelihood is: " << neg_lli_hood << "\n";
		return neg_lli_hood;

		//return (x[0]*x[0] + x[1]*x[1] + x[2]*x[2]);

	}

	Filter *filter;
	double *prxs, *estimates, *u, *v;
	int n;
	double p;

	void estimate_extended_kalman_parameters_1_dim(
	double *log_stock_prices,
	double muS,
	int n_stock_prices,
	double omega,
	double theta,
	double xi,
	double rho,
	double *u,
	double *v,
	double *estimates)
{
	int i1;
	double x, x1, W, H, A;
	double P, P1, z, U, K;
	double delt=1.0/252.0;
	double eps=0.00001;
	x = 0.04;
	P=0.01;
	u[0]=u[n_stock_prices-1]=0.0;
	v[0]=v[n_stock_prices-1]=1.0;
	estimates[0]=estimates[1]=log_stock_prices[0]+eps;
	for (i1=1;i1<n_stock_prices-1;i1++)
	{
		if (x<0) x=0.00001;
		x1 = x + ( omega-rho*xi*muS - (theta-0.5*rho*xi) * x) * delt +
			rho*xi* (log_stock_prices[i1]-log_stock_prices[i1-1]);
		A = 1.0-(theta-0.5*rho*xi)*delt;
		W = xi*sqrt((1-rho*rho) * x * delt);
		P1 = W*W + A*P*A;
		if (x1<0) x1=0.00001;
		H = -0.5*delt;
		U = sqrt(x1*delt);
		K = P1*H/( H*P1*H + U*U);
		z = log_stock_prices[i1+1];
		x = x1 + K * (z - (log_stock_prices[i1] + (muS-0.5*x1)*delt));
		u[i1] = z - (log_stock_prices[i1] + (muS-0.5*x1)*delt);
		v[i1] = H*P1*H + U*U;
		estimates[i1+1] = log_stock_prices[i1] + (muS-0.5*x1)*delt;
		P=(1.0-K*H)*P1;
	}
}

void estimate_extended_kalman_parameters_1_dim_fin_proj(
	double *log_stock_prices,
	double muS,
	int n_stock_prices,
	double p,
	double kappa,
	double bigV,
	double xi,
	double rho,
	double *u,
	double *v,
	double *estimates)
{
	int i1;
	double x, x1, W, H, A;
	double P, P1, z, U, K;
	double delt=1.0/252.0;
	double eps=0.00001;
	double omega;
	double theta;
	double pow_xi;
	omega = kappa*bigV; //K(V - v_k) = w - theta*v_k = theta(w/theta - v_k)
	theta = kappa;

	x = 0.04;
	P=0.01;
	u[0]=u[n_stock_prices-1]=0.0;
	v[0]=v[n_stock_prices-1]=1.0;
	estimates[0]=estimates[1]=log_stock_prices[0]+eps;

	if(omega < 0) omega = 0.2;
	if(theta < 0) theta = 8.0;
	if(rho > 1 || rho < -1 ) rho = -0.5;
	if(xi < 0) xi = 0.04;

	for (i1=1;i1<n_stock_prices-1;i1++)
	{
		if (x<0) x=0.00001;

		pow_xi = pow(x,p-0.5);

		x1 = x + ( omega - (theta-0.5*rho*xi*pow_xi) * x) * delt +
			rho*xi* pow_xi * (log_stock_prices[i1]-log_stock_prices[i1-1]); 
			// xi* sqrt( 1 - pow(rho,2.0)) * pow(x,p)* sqrt(delt) * ;

		A = 1.0- (theta-0.5*rho*xi*(p+0.5)*pow(x, (p-0.5) ))*delt 
			+ (p - 0.5)*rho*xi*pow(x, p - 1.5)*(log_stock_prices[i1]-log_stock_prices[i1-1]) ;

		W = xi*sqrt((1-rho*rho) * delt) * pow(x,p);
		
		P1 = W*W + A*P*A;
		
		if (x1<0) x1=0.00001;
	
		H = -0.5*delt;
		U = sqrt(x1*delt);
		K = P1*H/( H*P1*H + U*U);
		
		z = log_stock_prices[i1+1];
		x = x1 + K * (z - (log_stock_prices[i1] + (muS-0.5*x1)*delt));
		u[i1] = z - (log_stock_prices[i1] + (muS-0.5*x1)*delt);
		v[i1] = H*P1*H + U*U;
		estimates[i1+1] = log_stock_prices[i1] + (muS-0.5*x1)*delt;
		
		P=(1.0-K*H)*P1;
	}
}


};

Minimizer::Minimizer(void)
{
}


Minimizer::~Minimizer(void)
{
}

void Minimizer::estimate_params(Filter *filter_type, double *prxs, double *estimates, double *u, double *v, int n, double p) {

	std::default_random_engine generator;
	std::uniform_real_distribution<double> rho_dist(-0.1,0.0);
	std::uniform_real_distribution<double> kappa_dist(0.0,1.0);
	std::uniform_real_distribution<double> v_dist(0.0,100.0);
	std::uniform_real_distribution<double> xi_dist(0.0,5.0);


	cout << " estimating params\n";
	Func filter(filter_type, prxs, estimates, u, v, n, p);
	Int nr_of_params = 4;

	Int i;
    
	VecDoub pinit(nr_of_params , 0.0); 
	pinit[0]= 0.15;
	pinit[1]= 10.0;
	pinit[2]= 0.02;
	pinit[3]=-0.7;

	VecDoub pmin;
	Doub tol = 0.00000001;
	cout << "Calling powell minimization \n";

	Powell <Func> powell(filter, tol);
	pmin = powell.minimize(pinit);

	while (pmin[3] > 1.0 || pmin[3] < -1.0 || pmin[0] < 0.0) {
		cout << "Rho or Kappa is out of constrained range \n";
		pinit[0]= kappa_dist(generator);
		pinit[1]= v_dist(generator);
		pinit[2]= xi_dist(generator);
		pinit[3]=rho_dist(generator);
		pmin = powell.minimize(pinit);
	}

	cout << "Number of Powell iterations  =  " << powell.iter << endl << endl;
    cout << "After Powell, minimum value  = ";
    cout << setw(13) << powell.fret << " at: ";
    for (i = 0; i < nr_of_params; i++) {
        cout << setw(13) << pmin[i] << " ";
    }
    cout << endl << endl;

};

