/* t-mpi-point.c  - Tests for mpi point functions
 * Copyright (C) 2013 g10 Code GmbH
 *
 * This file is part of Libgcrypt.
 *
 * Libgcrypt is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * Libgcrypt is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>

#include "../src/gcrypt-int.h"

#define PGM "t-mpi-point"

static const char *wherestr;
static int verbose;
static int debug;
static int error_count;


#define xmalloc(a)    gcry_xmalloc ((a))
#define xcalloc(a,b)  gcry_xcalloc ((a),(b))
#define xfree(a)      gcry_free ((a))
#define pass() do { ; } while (0)


static struct
{
  const char *desc;           /* Description of the curve.  */
  const char *p;              /* Order of the prime field.  */
  const char *a, *b;          /* The coefficients. */
  const char *n;              /* The order of the base point.  */
  const char *g_x, *g_y;      /* Base point.  */
} test_curve[] =
  {
    {
      "NIST P-192",
      "0xfffffffffffffffffffffffffffffffeffffffffffffffff",
      "0xfffffffffffffffffffffffffffffffefffffffffffffffc",
      "0x64210519e59c80e70fa7e9ab72243049feb8deecc146b9b1",
      "0xffffffffffffffffffffffff99def836146bc9b1b4d22831",

      "0x188da80eb03090f67cbf20eb43a18800f4ff0afd82ff1012",
      "0x07192b95ffc8da78631011ed6b24cdd573f977a11e794811"
    },
    {
      "NIST P-224",
      "0xffffffffffffffffffffffffffffffff000000000000000000000001",
      "0xfffffffffffffffffffffffffffffffefffffffffffffffffffffffe",
      "0xb4050a850c04b3abf54132565044b0b7d7bfd8ba270b39432355ffb4",
      "0xffffffffffffffffffffffffffff16a2e0b8f03e13dd29455c5c2a3d" ,

      "0xb70e0cbd6bb4bf7f321390b94a03c1d356c21122343280d6115c1d21",
      "0xbd376388b5f723fb4c22dfe6cd4375a05a07476444d5819985007e34"
    },
    {
      "NIST P-256",
      "0xffffffff00000001000000000000000000000000ffffffffffffffffffffffff",
      "0xffffffff00000001000000000000000000000000fffffffffffffffffffffffc",
      "0x5ac635d8aa3a93e7b3ebbd55769886bc651d06b0cc53b0f63bce3c3e27d2604b",
      "0xffffffff00000000ffffffffffffffffbce6faada7179e84f3b9cac2fc632551",

      "0x6b17d1f2e12c4247f8bce6e563a440f277037d812deb33a0f4a13945d898c296",
      "0x4fe342e2fe1a7f9b8ee7eb4a7c0f9e162bce33576b315ececbb6406837bf51f5"
    },
    {
      "NIST P-384",
      "0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe"
      "ffffffff0000000000000000ffffffff",
      "0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe"
      "ffffffff0000000000000000fffffffc",
      "0xb3312fa7e23ee7e4988e056be3f82d19181d9c6efe8141120314088f5013875a"
      "c656398d8a2ed19d2a85c8edd3ec2aef",
      "0xffffffffffffffffffffffffffffffffffffffffffffffffc7634d81f4372ddf"
      "581a0db248b0a77aecec196accc52973",

      "0xaa87ca22be8b05378eb1c71ef320ad746e1d3b628ba79b9859f741e082542a38"
      "5502f25dbf55296c3a545e3872760ab7",
      "0x3617de4a96262c6f5d9e98bf9292dc29f8f41dbd289a147ce9da3113b5f0b8c0"
      "0a60b1ce1d7e819d7a431d7c90ea0e5f"
    },
    {
      "NIST P-521",
      "0x01ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"
      "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff",
      "0x01ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"
      "fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffc",
      "0x051953eb9618e1c9a1f929a21a0b68540eea2da725b99b315f3b8b489918ef10"
      "9e156193951ec7e937b1652c0bd3bb1bf073573df883d2c34f1ef451fd46b503f00",
      "0x1fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"
      "ffa51868783bf2f966b7fcc0148f709a5d03bb5c9b8899c47aebb6fb71e91386409",

      "0xc6858e06b70404e9cd9e3ecb662395b4429c648139053fb521f828af606b4d3d"
      "baa14b5e77efe75928fe1dc127a2ffa8de3348b3c1856a429bf97e7e31c2e5bd66",
      "0x11839296a789a3bc0045c8a5fb42c7d1bd998f54449579b446817afbd17273e6"
      "62c97ee72995ef42640c550b9013fad0761353c7086a272c24088be94769fd16650"
    },
    { NULL, NULL, NULL, NULL, NULL }
  };

/* A sample public key for NIST P-256.  */
static const char sample_p256_q[] =
  "04"
  "42B927242237639A36CE9221B340DB1A9AB76DF2FE3E171277F6A4023DED146E"
  "E86525E38CCECFF3FB8D152CC6334F70D23A525175C1BCBDDE6E023B2228770E";
static const char sample_p256_q_x[] =
  "42B927242237639A36CE9221B340DB1A9AB76DF2FE3E171277F6A4023DED146E";
static const char sample_p256_q_y[] =
  "00E86525E38CCECFF3FB8D152CC6334F70D23A525175C1BCBDDE6E023B2228770E";



static void
show (const char *format, ...)
{
  va_list arg_ptr;

  if (!verbose)
    return;
  fprintf (stderr, "%s: ", PGM);
  va_start (arg_ptr, format);
  vfprintf (stderr, format, arg_ptr);
  va_end (arg_ptr);
}

static void
fail (const char *format, ...)
{
  va_list arg_ptr;

  fflush (stdout);
  fprintf (stderr, "%s: ", PGM);
  if (wherestr)
    fprintf (stderr, "%s: ", wherestr);
  va_start (arg_ptr, format);
  vfprintf (stderr, format, arg_ptr);
  va_end (arg_ptr);
  error_count++;
}

static void
die (const char *format, ...)
{
  va_list arg_ptr;

  fflush (stdout);
  fprintf (stderr, "%s: ", PGM);
  if (wherestr)
    fprintf (stderr, "%s: ", wherestr);
  va_start (arg_ptr, format);
  vfprintf (stderr, format, arg_ptr);
  va_end (arg_ptr);
  exit (1);
}


static void
print_mpi_2 (const char *text, const char *text2, gcry_mpi_t a)
{
  gcry_error_t err;
  char *buf;
  void *bufaddr = &buf;

  err = gcry_mpi_aprint (GCRYMPI_FMT_HEX, bufaddr, NULL, a);
  if (err)
    fprintf (stderr, "%s%s: [error printing number: %s]\n",
             text, text2? text2:"", gpg_strerror (err));
  else
    {
      fprintf (stderr, "%s%s: %s\n", text, text2? text2:"", buf);
      gcry_free (buf);
    }
}


static void
print_mpi (const char *text, gcry_mpi_t a)
{
  print_mpi_2 (text, NULL, a);
}


static void
print_point (const char *text, gcry_mpi_point_t a)
{
  gcry_mpi_t x, y, z;

  x = gcry_mpi_new (0);
  y = gcry_mpi_new (0);
  z = gcry_mpi_new (0);
  gcry_mpi_point_get (x, y, z, a);
  print_mpi_2 (text, ".x", x);
  print_mpi_2 (text, ".y", y);
  print_mpi_2 (text, ".z", z);
  gcry_mpi_release (x);
  gcry_mpi_release (y);
  gcry_mpi_release (z);
}


static void
print_sexp (const char *prefix, gcry_sexp_t a)
{
  char *buf;
  size_t size;

  if (prefix)
    fputs (prefix, stderr);
  size = gcry_sexp_sprint (a, GCRYSEXP_FMT_ADVANCED, NULL, 0);
  buf = gcry_xmalloc (size);

  gcry_sexp_sprint (a, GCRYSEXP_FMT_ADVANCED, buf, size);
  fprintf (stderr, "%.*s", (int)size, buf);
  gcry_free (buf);
}


static gcry_mpi_t
hex2mpi (const char *string)
{
  gpg_error_t err;
  gcry_mpi_t val;

  err = gcry_mpi_scan (&val, GCRYMPI_FMT_HEX, string, 0, NULL);
  if (err)
    die ("hex2mpi '%s' failed: %s\n", gpg_strerror (err));
  return val;
}


/* Compare A to B, where B is given as a hex string.  */
static int
cmp_mpihex (gcry_mpi_t a, const char *b)
{
  gcry_mpi_t bval;
  int res;

  bval = hex2mpi (b);
  res = gcry_mpi_cmp (a, bval);
  gcry_mpi_release (bval);
  return res;
}


/* Wrapper to emulate the libgcrypt internal EC context allocation
   function.  */
static gpg_error_t
ec_p_new (gcry_ctx_t *r_ctx, gcry_mpi_t p, gcry_mpi_t a)
{
  gpg_error_t err;
  gcry_sexp_t sexp;

  if (p && a)
    err = gcry_sexp_build (&sexp, NULL, "(ecdsa (p %m)(a %m))", p, a);
  else if (p)
    err = gcry_sexp_build (&sexp, NULL, "(ecdsa (p %m))", p);
  else if (a)
    err = gcry_sexp_build (&sexp, NULL, "(ecdsa (a %m))", a);
  else
    err = gcry_sexp_build (&sexp, NULL, "(ecdsa)");
  if (err)
    return err;
  err = gcry_mpi_ec_new (r_ctx, sexp, NULL);
  gcry_sexp_release (sexp);
  return err;
}



static void
set_get_point (void)
{
  gcry_mpi_point_t point;
  gcry_mpi_t x, y, z;

  wherestr = "set_get_point";
  show ("checking point setting functions\n");

  point = gcry_mpi_point_new (0);
  x = gcry_mpi_set_ui (NULL, 17);
  y = gcry_mpi_set_ui (NULL, 42);
  z = gcry_mpi_set_ui (NULL, 11371);
  gcry_mpi_point_get (x, y, z, point);
  if (gcry_mpi_cmp_ui (x, 0)
      || gcry_mpi_cmp_ui (y, 0) || gcry_mpi_cmp_ui (z, 0))
    fail ("new point not initialized to (0,0,0)\n");
  gcry_mpi_point_snatch_get (x, y, z, point);
  point = NULL;
  if (gcry_mpi_cmp_ui (x, 0)
      || gcry_mpi_cmp_ui (y, 0) || gcry_mpi_cmp_ui (z, 0))
    fail ("snatch_get failed\n");
  gcry_mpi_release (x);
  gcry_mpi_release (y);
  gcry_mpi_release (z);

  point = gcry_mpi_point_new (0);
  x = gcry_mpi_set_ui (NULL, 17);
  y = gcry_mpi_set_ui (NULL, 42);
  z = gcry_mpi_set_ui (NULL, 11371);
  gcry_mpi_point_set (point, x, y, z);
  gcry_mpi_set_ui (x, 23);
  gcry_mpi_set_ui (y, 24);
  gcry_mpi_set_ui (z, 25);
  gcry_mpi_point_get (x, y, z, point);
  if (gcry_mpi_cmp_ui (x, 17)
      || gcry_mpi_cmp_ui (y, 42) || gcry_mpi_cmp_ui (z, 11371))
    fail ("point_set/point_get failed\n");
  gcry_mpi_point_snatch_set (point, x, y, z);
  x = gcry_mpi_new (0);
  y = gcry_mpi_new (0);
  z = gcry_mpi_new (0);
  gcry_mpi_point_get (x, y, z, point);
  if (gcry_mpi_cmp_ui (x, 17)
      || gcry_mpi_cmp_ui (y, 42) || gcry_mpi_cmp_ui (z, 11371))
    fail ("point_snatch_set/point_get failed\n");

  gcry_mpi_point_release (point);
  gcry_mpi_release (x);
  gcry_mpi_release (y);
  gcry_mpi_release (z);
}


static void
context_alloc (void)
{
  gpg_error_t err;
  gcry_ctx_t ctx;
  gcry_mpi_t p, a;

  wherestr = "context_alloc";
  show ("checking context functions\n");

  p = gcry_mpi_set_ui (NULL, 1);
  a = gcry_mpi_set_ui (NULL, 1);
  err = ec_p_new (&ctx, p, a);
  if (err)
    die ("ec_p_new returned an error: %s\n", gpg_strerror (err));
  gcry_mpi_release (p);
  gcry_mpi_release (a);
  gcry_ctx_release (ctx);

  p = gcry_mpi_set_ui (NULL, 0);
  a = gcry_mpi_set_ui (NULL, 0);
  err = ec_p_new (&ctx, p, a);
  if (!err || gpg_err_code (err) != GPG_ERR_EINVAL)
    fail ("ec_p_new: bad parameter detection failed (1)\n");

  gcry_mpi_set_ui (p, 1);
  err = ec_p_new (&ctx, p, a);
  if (!err || gpg_err_code (err) != GPG_ERR_EINVAL)
    fail ("ec_p_new: bad parameter detection failed (2)\n");

  gcry_mpi_release (p);
  p = NULL;
  err = ec_p_new (&ctx, p, a);
  if (!err || gpg_err_code (err) != GPG_ERR_EINVAL)
    fail ("ec_p_new: bad parameter detection failed (3)\n");

  gcry_mpi_release (a);
  a = NULL;
  err = ec_p_new (&ctx, p, a);
  if (!err || gpg_err_code (err) != GPG_ERR_EINVAL)
    fail ("ec_p_new: bad parameter detection failed (4)\n");

}


static int
get_and_cmp_mpi (const char *name, const char *mpistring, const char *desc,
                 gcry_ctx_t ctx)
{
  gcry_mpi_t mpi;

  mpi = gcry_mpi_ec_get_mpi (name, ctx, 1);
  if (!mpi)
    {
      fail ("error getting parameter '%s' of curve '%s'\n", name, desc);
      return 1;
    }
  if (debug)
    print_mpi (name, mpi);
  if (cmp_mpihex (mpi, mpistring))
    {
      fail ("parameter '%s' of curve '%s' does not match\n", name, desc);
      gcry_mpi_release (mpi);
      return 1;
    }
  gcry_mpi_release (mpi);
  return 0;
}


static int
get_and_cmp_point (const char *name,
                   const char *mpi_x_string, const char *mpi_y_string,
                   const char *desc, gcry_ctx_t ctx)
{
  gcry_mpi_point_t point;
  gcry_mpi_t x, y, z;
  int result = 0;

  point = gcry_mpi_ec_get_point (name, ctx, 1);
  if (!point)
    {
      fail ("error getting point parameter '%s' of curve '%s'\n", name, desc);
      return 1;
    }
  if (debug)
    print_point (name, point);

  x = gcry_mpi_new (0);
  y = gcry_mpi_new (0);
  z = gcry_mpi_new (0);
  gcry_mpi_point_snatch_get (x, y, z, point);
  if (cmp_mpihex (x, mpi_x_string))
    {
      fail ("x coordinate of '%s' of curve '%s' does not match\n", name, desc);
      result = 1;
    }
  if (cmp_mpihex (y, mpi_y_string))
    {
      fail ("y coordinate of '%s' of curve '%s' does not match\n", name, desc);
      result = 1;
    }
  if (cmp_mpihex (z, "01"))
    {
      fail ("z coordinate of '%s' of curve '%s' is not 1\n", name, desc);
      result = 1;
    }
  gcry_mpi_release (x);
  gcry_mpi_release (y);
  gcry_mpi_release (z);
  return result;
}


static void
context_param (void)
{
  gpg_error_t err;
  int idx;
  gcry_ctx_t ctx = NULL;
  gcry_mpi_t q;
  gcry_sexp_t keyparam;

  wherestr = "context_param";

  show ("checking standard curves\n");
  for (idx=0; test_curve[idx].desc; idx++)
    {
      gcry_ctx_release (ctx);
      err = gcry_mpi_ec_new (&ctx, NULL, test_curve[idx].desc);
      if (err)
        {
          fail ("can't create context for curve '%s': %s\n",
                test_curve[idx].desc, gpg_strerror (err));
          continue;
        }
      if (get_and_cmp_mpi ("p", test_curve[idx].p, test_curve[idx].desc, ctx))
        continue;
      if (get_and_cmp_mpi ("a", test_curve[idx].a, test_curve[idx].desc, ctx))
        continue;
      if (get_and_cmp_mpi ("b", test_curve[idx].b, test_curve[idx].desc, ctx))
        continue;
      if (get_and_cmp_mpi ("g.x",test_curve[idx].g_x, test_curve[idx].desc,ctx))
        continue;
      if (get_and_cmp_mpi ("g.y",test_curve[idx].g_y, test_curve[idx].desc,ctx))
        continue;
      if (get_and_cmp_mpi ("n", test_curve[idx].n, test_curve[idx].desc, ctx))
        continue;
      if (get_and_cmp_point ("g", test_curve[idx].g_x, test_curve[idx].g_y,
                             test_curve[idx].desc, ctx))
        continue;

    }
  gcry_ctx_release (ctx);


  show ("checking sample public key\n");
  q = hex2mpi (sample_p256_q);
  err = gcry_sexp_build (&keyparam, NULL,
                        "(public-key(ecdsa(curve %s)(q %m)))",
                        "NIST P-256", q);
  if (err)
    die ("gcry_sexp_build failed: %s\n", gpg_strerror (err));
  gcry_mpi_release (q);

  /* We can't call gcry_pk_testkey because it is only implemented for
     private keys.  */
  /* err = gcry_pk_testkey (keyparam); */
  /* if (err) */
  /*   fail ("gcry_pk_testkey failed for sample public key: %s\n", */
  /*         gpg_strerror (err)); */

  err = gcry_mpi_ec_new (&ctx, keyparam, NULL);
  if (err)
    fail ("gcry_mpi_ec_new failed for sample public key: %s\n",
          gpg_strerror (err));
  else
    {
      gcry_sexp_t sexp;

      get_and_cmp_mpi ("q", sample_p256_q, "NIST P-256", ctx);
      get_and_cmp_point ("q", sample_p256_q_x, sample_p256_q_y, "NIST P-256",
                         ctx);

      err = gcry_pubkey_get_sexp (&sexp, 0, ctx);
      if (err)
        fail ("gcry_pubkey_get_sexp(0) failed: %s\n", gpg_strerror (err));
      else if (debug)
        print_sexp ("Result of gcry_pubkey_get_sexp (0):\n", sexp);
      gcry_sexp_release (sexp);

      err = gcry_pubkey_get_sexp (&sexp, GCRY_PK_GET_PUBKEY, ctx);
      if (err)
        fail ("gcry_pubkey_get_sexp(GET_PUBKEY) failed: %s\n",
              gpg_strerror (err));
      else if (debug)
        print_sexp ("Result of gcry_pubkey_get_sexp (GET_PUBKEY):\n", sexp);
      gcry_sexp_release (sexp);

      err = gcry_pubkey_get_sexp (&sexp, GCRY_PK_GET_SECKEY, ctx);
      if (gpg_err_code (err) != GPG_ERR_NO_SECKEY)
        fail ("gcry_pubkey_get_sexp(GET_SECKEY) returned wrong error: %s\n",
              gpg_strerror (err));
      gcry_sexp_release (sexp);

      gcry_ctx_release (ctx);
    }

  gcry_sexp_release (keyparam);
}




/* Create a new point from (X,Y,Z) given as hex strings.  */
gcry_mpi_point_t
make_point (const char *x, const char *y, const char *z)
{
  gcry_mpi_point_t point;

  point = gcry_mpi_point_new (0);
  gcry_mpi_point_snatch_set (point, hex2mpi (x), hex2mpi (y), hex2mpi (z));

  return point;
}


/* This tests checks that the low-level EC API yields the same result
   as using the high level API.  The values have been taken from a
   test run using the high level API.  */
static void
basic_ec_math (void)
{
  gpg_error_t err;
  gcry_ctx_t ctx;
  gcry_mpi_t P, A;
  gcry_mpi_point_t G, Q;
  gcry_mpi_t d;
  gcry_mpi_t x, y, z;

  wherestr = "basic_ec_math";
  show ("checking basic math functions for EC\n");

  P = hex2mpi ("0xfffffffffffffffffffffffffffffffeffffffffffffffff");
  A = hex2mpi ("0xfffffffffffffffffffffffffffffffefffffffffffffffc");
  G = make_point ("188DA80EB03090F67CBF20EB43A18800F4FF0AFD82FF1012",
                  "7192B95FFC8DA78631011ED6B24CDD573F977A11E794811",
                  "1");
  d = hex2mpi ("D4EF27E32F8AD8E2A1C6DDEBB1D235A69E3CEF9BCE90273D");
  Q = gcry_mpi_point_new (0);

  err = ec_p_new (&ctx, P, A);
  if (err)
    die ("ec_p_new failed: %s\n", gpg_strerror (err));

  x = gcry_mpi_new (0);
  y = gcry_mpi_new (0);
  z = gcry_mpi_new (0);

  {
    /* A quick check that multiply by zero works.  */
    gcry_mpi_t tmp;

    tmp = gcry_mpi_new (0);
    gcry_mpi_ec_mul (Q, tmp, G, ctx);
    gcry_mpi_release (tmp);
    gcry_mpi_point_get (x, y, z, Q);
    if (gcry_mpi_cmp_ui (x, 0) || gcry_mpi_cmp_ui (y, 0)
        || gcry_mpi_cmp_ui (z, 0))
      fail ("multiply a point by zero failed\n");
  }

  gcry_mpi_ec_mul (Q, d, G, ctx);
  gcry_mpi_point_get (x, y, z, Q);
  if (cmp_mpihex (x, "222D9EC717C89D047E0898C9185B033CD11C0A981EE6DC66")
      || cmp_mpihex (y, "605DE0A82D70D3E0F84A127D0739ED33D657DF0D054BFDE8")
      || cmp_mpihex (z, "00B06B519071BC536999AC8F2D3934B3C1FC9EACCD0A31F88F"))
    fail ("computed public key does not match\n");
  if (debug)
    {
      print_mpi ("Q.x", x);
      print_mpi ("Q.y", y);
      print_mpi ("Q.z", z);
    }

  if (gcry_mpi_ec_get_affine (x, y, Q, ctx))
    fail ("failed to get affine coordinates\n");
  if (cmp_mpihex (x, "008532093BA023F4D55C0424FA3AF9367E05F309DC34CDC3FE")
      || cmp_mpihex (y, "00C13CA9E617C6C8487BFF6A726E3C4F277913D97117939966"))
    fail ("computed affine coordinates of public key do not match\n");
  if (debug)
    {
      print_mpi ("q.x", x);
      print_mpi ("q.y", y);
    }

  gcry_mpi_release (z);
  gcry_mpi_release (y);
  gcry_mpi_release (x);
  gcry_mpi_point_release (Q);
  gcry_mpi_release (d);
  gcry_mpi_point_release (G);
  gcry_mpi_release (A);
  gcry_mpi_release (P);
  gcry_ctx_release (ctx);
}


/* This is the same as basic_ec_math but uses more advanced
   features.  */
static void
basic_ec_math_simplified (void)
{
  gpg_error_t err;
  gcry_ctx_t ctx;
  gcry_mpi_point_t G, Q;
  gcry_mpi_t d;
  gcry_mpi_t x, y, z;
  gcry_sexp_t sexp;

  wherestr = "basic_ec_math_simplified";
  show ("checking basic math functions for EC (variant)\n");

  d = hex2mpi ("D4EF27E32F8AD8E2A1C6DDEBB1D235A69E3CEF9BCE90273D");
  Q = gcry_mpi_point_new (0);

  err = gcry_mpi_ec_new (&ctx, NULL, "NIST P-192");
  if (err)
    die ("gcry_mpi_ec_new failed: %s\n", gpg_strerror (err));
  G = gcry_mpi_ec_get_point ("g", ctx, 1);
  if (!G)
    die ("gcry_mpi_ec_get_point(G) failed\n");
  gcry_mpi_ec_mul (Q, d, G, ctx);

  x = gcry_mpi_new (0);
  y = gcry_mpi_new (0);
  z = gcry_mpi_new (0);
  gcry_mpi_point_get (x, y, z, Q);
  if (cmp_mpihex (x, "222D9EC717C89D047E0898C9185B033CD11C0A981EE6DC66")
      || cmp_mpihex (y, "605DE0A82D70D3E0F84A127D0739ED33D657DF0D054BFDE8")
      || cmp_mpihex (z, "00B06B519071BC536999AC8F2D3934B3C1FC9EACCD0A31F88F"))
    fail ("computed public key does not match\n");
  if (debug)
    {
      print_mpi ("Q.x", x);
      print_mpi ("Q.y", y);
      print_mpi ("Q.z", z);
    }

  if (gcry_mpi_ec_get_affine (x, y, Q, ctx))
    fail ("failed to get affine coordinates\n");
  if (cmp_mpihex (x, "008532093BA023F4D55C0424FA3AF9367E05F309DC34CDC3FE")
      || cmp_mpihex (y, "00C13CA9E617C6C8487BFF6A726E3C4F277913D97117939966"))
    fail ("computed affine coordinates of public key do not match\n");
  if (debug)
    {
      print_mpi ("q.x", x);
      print_mpi ("q.y", y);
    }

  gcry_mpi_release (z);
  gcry_mpi_release (y);
  gcry_mpi_release (x);

  /* Let us also check wheer we can update the context.  */
  err = gcry_mpi_ec_set_point ("g", G, ctx);
  if (err)
    die ("gcry_mpi_ec_set_point(G) failed\n");
  err = gcry_mpi_ec_set_mpi ("d", d, ctx);
  if (err)
    die ("gcry_mpi_ec_set_mpi(d) failed\n");

  /* FIXME: Below we need to check that the returned S-expression is
     as requested.  For now we use manual inspection using --debug.  */

  /* Does get_sexp return the private key?  */
  err = gcry_pubkey_get_sexp (&sexp, 0, ctx);
  if (err)
    fail ("gcry_pubkey_get_sexp(0) failed: %s\n", gpg_strerror (err));
  else if (verbose)
    print_sexp ("Result of gcry_pubkey_get_sexp (0):\n", sexp);
  gcry_sexp_release (sexp);

  /* Does get_sexp return the public key if requested?  */
  err = gcry_pubkey_get_sexp (&sexp, GCRY_PK_GET_PUBKEY, ctx);
  if (err)
    fail ("gcry_pubkey_get_sexp(GET_PUBKEY) failed: %s\n", gpg_strerror (err));
  else if (verbose)
    print_sexp ("Result of gcry_pubkey_get_sexp (GET_PUBKEY):\n", sexp);
  gcry_sexp_release (sexp);

  /* Does get_sexp return the public key if after d has been deleted?  */
  err = gcry_mpi_ec_set_mpi ("d", NULL, ctx);
  if (err)
    die ("gcry_mpi_ec_set_mpi(d=NULL) failed\n");
  err = gcry_pubkey_get_sexp (&sexp, 0, ctx);
  if (err)
    fail ("gcry_pubkey_get_sexp(0 w/o d) failed: %s\n", gpg_strerror (err));
  else if (verbose)
    print_sexp ("Result of gcry_pubkey_get_sexp (0 w/o d):\n", sexp);
  gcry_sexp_release (sexp);

  /* Does get_sexp return an error after d has been deleted?  */
  err = gcry_pubkey_get_sexp (&sexp, GCRY_PK_GET_SECKEY, ctx);
  if (gpg_err_code (err) != GPG_ERR_NO_SECKEY)
    fail ("gcry_pubkey_get_sexp(GET_SECKEY) returned wrong error: %s\n",
          gpg_strerror (err));
  gcry_sexp_release (sexp);

  /* Does get_sexp return an error after d and Q have been deleted?  */
  err = gcry_mpi_ec_set_point ("q", NULL, ctx);
  if (err)
    die ("gcry_mpi_ec_set_point(q=NULL) failed\n");
  err = gcry_pubkey_get_sexp (&sexp, 0, ctx);
  if (gpg_err_code (err) != GPG_ERR_BAD_CRYPT_CTX)
    fail ("gcry_pubkey_get_sexp(0 w/o Q,d) returned wrong error: %s\n",
          gpg_strerror (err));
  gcry_sexp_release (sexp);


  gcry_mpi_point_release (Q);
  gcry_mpi_release (d);
  gcry_mpi_point_release (G);
  gcry_ctx_release (ctx);
}


int
main (int argc, char **argv)
{

  if (argc > 1 && !strcmp (argv[1], "--verbose"))
    verbose = 1;
  else if (argc > 1 && !strcmp (argv[1], "--debug"))
    verbose = debug = 1;

  if (!gcry_check_version (GCRYPT_VERSION))
    die ("version mismatch\n");

  gcry_control (GCRYCTL_DISABLE_SECMEM, 0);
  gcry_control (GCRYCTL_ENABLE_QUICK_RANDOM, 0);
  if (debug)
    gcry_control (GCRYCTL_SET_DEBUG_FLAGS, 1u, 0);
  gcry_control (GCRYCTL_INITIALIZATION_FINISHED, 0);

  set_get_point ();
  context_alloc ();
  context_param ();
  basic_ec_math ();
  basic_ec_math_simplified ();

  show ("All tests completed. Errors: %d\n", error_count);
  return error_count ? 1 : 0;
}