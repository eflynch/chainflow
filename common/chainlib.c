/* This file is hardcoded to Tidmarsh Unity */

#define X_SCALE 83459.2085
#define Z_SCALE 109938.8055
#define X_OFFSET 5890083.394
#define Z_OFFSET -4606524.694

void chain_convert_geo_xz(double lat, double lon, double *result)
{
   result[0] = lon * X_SCALE + X_OFFSET;
   result[1] = lat * Z_SCALE + Z_OFFSET; 
}

void chain_convert_xz_geo(double x, double z, double *result)
{
    result[0] = (z - Z_OFFSET) / Z_SCALE;
    result[1] = (x - X_OFFSET) / X_SCALE;
}
