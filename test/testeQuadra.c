#include "unity.h"
#include "quadra.h"

void setUp(void) {}
void tearDown(void) {}

static void test_quadra_criar_e_ler_campos(void) {
    Quadra q = quadra_criar("ABC123", 10.0, 20.0, 30.0, 40.0, "yellow", "black", 2.0);

    TEST_ASSERT_NOT_NULL(q);
    TEST_ASSERT_EQUAL_STRING("ABC123", quadra_get_cep(q));
    TEST_ASSERT_DOUBLE_WITHIN(0.0001, 10.0, quadra_get_x(q));
    TEST_ASSERT_DOUBLE_WITHIN(0.0001, 20.0, quadra_get_y(q));
    TEST_ASSERT_DOUBLE_WITHIN(0.0001, 30.0, quadra_get_w(q));
    TEST_ASSERT_DOUBLE_WITHIN(0.0001, 40.0, quadra_get_h(q));
    TEST_ASSERT_EQUAL_STRING("yellow", quadra_get_cfill(q));
    TEST_ASSERT_EQUAL_STRING("black", quadra_get_cstrk(q));
    TEST_ASSERT_DOUBLE_WITHIN(0.0001, 2.0, quadra_get_sw(q));

    quadra_destruir(q);
}

static void test_calcular_coordenada_endereco_por_face(void) {
    double x;
    double y;

    calcular_coordenada_endereco(10.0, 20.0, 100.0, 40.0, 'S', 15, &x, &y);
    TEST_ASSERT_DOUBLE_WITHIN(0.0001, 25.0, x);
    TEST_ASSERT_DOUBLE_WITHIN(0.0001, 20.0, y);

    calcular_coordenada_endereco(10.0, 20.0, 100.0, 40.0, 'N', 15, &x, &y);
    TEST_ASSERT_DOUBLE_WITHIN(0.0001, 25.0, x);
    TEST_ASSERT_DOUBLE_WITHIN(0.0001, 60.0, y);

    calcular_coordenada_endereco(10.0, 20.0, 100.0, 40.0, 'L', 15, &x, &y);
    TEST_ASSERT_DOUBLE_WITHIN(0.0001, 10.0, x);
    TEST_ASSERT_DOUBLE_WITHIN(0.0001, 35.0, y);

    calcular_coordenada_endereco(10.0, 20.0, 100.0, 40.0, 'O', 15, &x, &y);
    TEST_ASSERT_DOUBLE_WITHIN(0.0001, 110.0, x);
    TEST_ASSERT_DOUBLE_WITHIN(0.0001, 35.0, y);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_quadra_criar_e_ler_campos);
    RUN_TEST(test_calcular_coordenada_endereco_por_face);
    return UNITY_END();
}
