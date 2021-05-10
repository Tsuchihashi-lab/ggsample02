// ウィンドウ関連の処理
#include "Window.h"

//
// シェーダオブジェクトのコンパイル結果を表示する
//
//   shader: シェーダオブジェクト名
//   str: コンパイルエラーが発生した場所を示す文字列
//   戻り値: コンパイルに成功していたら GL_TRUE
//
static GLboolean printShaderInfoLog(GLuint shader, const char* str)
{
  // コンパイル結果を取得する
  GLint status;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
  if (status == GL_FALSE) std::cerr << "Compile Error in " << str << std::endl;

  // シェーダのコンパイル時のログの長さを取得する
  GLsizei bufSize;
  glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &bufSize);

  if (bufSize > 1)
  {
    // シェーダのコンパイル時のログの内容を取得する
    std::vector<GLchar> infoLog(bufSize);
    GLsizei length;
    glGetShaderInfoLog(shader, bufSize, &length, &infoLog[0]);
    std::cerr << &infoLog[0] << std::endl;
  }

  return static_cast<GLboolean>(status);
}

//
// プログラムオブジェクトのリンク結果を表示する
//
//   program: プログラムオブジェクト名
//   戻り値: リンクに成功していたら GL_TRUE
//
static GLboolean printProgramInfoLog(GLuint program)
{
  // リンク結果を取得する
  GLint status;
  glGetProgramiv(program, GL_LINK_STATUS, &status);
  if (status == GL_FALSE) std::cerr << "Link Error." << std::endl;

  // シェーダのリンク時のログの長さを取得する
  GLsizei bufSize;
  glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufSize);

  if (bufSize > 1)
  {
    // シェーダのリンク時のログの内容を取得する
    std::vector<GLchar> infoLog(bufSize);
    GLsizei length;
    glGetProgramInfoLog(program, bufSize, &length, &infoLog[0]);
    std::cerr << &infoLog[0] << std::endl;
  }

  return static_cast<GLboolean>(status);
}

//
// プログラムオブジェクトを作成する
//
//   vsrc: バーテックスシェーダのソースプログラムの文字列
//   pv: バーテックスシェーダのソースプログラム中の in 変数名の文字列
//   fsrc: フラグメントシェーダのソースプログラムの文字列
//   fc: フラグメントシェーダのソースプログラム中の out 変数名の文字列
//   vert: バーテックスシェーダのコンパイル時のメッセージに追加する文字列
//   frag: フラグメントシェーダのコンパイル時のメッセージに追加する文字列
//   戻り値: プログラムオブジェクト名
//
static GLuint createProgram(const char* vsrc, const char* pv, const char* fsrc, const char* fc,
  const char* vert = "vertex shader", const char* frag = "fragment shader")
{
  // 空のプログラムオブジェクトを作成する
  const GLuint program(glCreateProgram());

  if (vsrc != NULL)
  {
    // バーテックスシェーダのシェーダオブジェクトを作成する
    const GLuint vobj(glCreateShader(GL_VERTEX_SHADER));
    glShaderSource(vobj, 1, &vsrc, NULL);
    glCompileShader(vobj);

    // バーテックスシェーダのシェーダオブジェクトをプログラムオブジェクトに組み込む
    if (printShaderInfoLog(vobj, vert))
      glAttachShader(program, vobj);
    glDeleteShader(vobj);
  }

  if (fsrc != NULL)
  {
    // フラグメントシェーダのシェーダオブジェクトを作成する
    const GLuint fobj(glCreateShader(GL_FRAGMENT_SHADER));
    glShaderSource(fobj, 1, &fsrc, NULL);
    glCompileShader(fobj);

    // フラグメントシェーダのシェーダオブジェクトをプログラムオブジェクトに組み込む
    if (printShaderInfoLog(fobj, vert))
      glAttachShader(program, fobj);
    glDeleteShader(fobj);
  }

  // プログラムオブジェクトをリンクする
  glBindAttribLocation(program, 0, pv);
  glBindFragDataLocation(program, 0, fc);
  glLinkProgram(program);

  // 作成したプログラムオブジェクトを返す
  if (printProgramInfoLog(program))
    return program;

  // プログラムオブジェクトが作成できなければ 0 を返す
  glDeleteProgram(program);
  return 0;
}

//
// 頂点配列オブジェクトの作成
//
//   vertices: 頂点数
//   position: 頂点の二次元位置 (GLfloat[2] の配列)
//   lines: 線分数
//   index: 線分の頂点インデックス
//   戻り値: 作成された頂点配列オブジェクト名
//
static GLuint createObject(GLuint vertices, const GLfloat(*position)[3], GLuint lines, const GLuint* index)
{
  // 頂点配列オブジェクト
  GLuint vao;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  // 頂点バッファオブジェクト
  GLuint vbo;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat[3]) * vertices, position, GL_STATIC_DRAW);

  // インデックスバッファオブジェクト
  GLuint ibo;
  glGenBuffers(1, &ibo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * lines, index, GL_STATIC_DRAW);

  // 結合されている頂点バッファオブジェクトを in 変数から参照できるようにする
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(0);

  // 頂点配列オブジェクトの結合を解除した後に頂点バッファオブジェクトとインデックスバッファオブジェクトの結合を解除する
  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  return vao;
}

//
// アプリケーションの実行
//
void app()
{
  // ウィンドウを作成する
  Window window("ggsample02");

  // 背景色を指定する
  glClearColor(1.0f, 1.0f, 1.0f, 0.0f);

  // バーテックスシェーダのソースプログラム
  static const GLchar vsrc[] =
    "#version 410 core\n"
    "in vec4 pv;\n"
    "void main(void)\n"
    "{\n"
    "  gl_Position = pv;\n"
    "}\n";

  // フラグメントシェーダのソースプログラム
  static const GLchar fsrc[] =
    "#version 410 core\n"
    "out vec4 fc;\n"
    "void main(void)\n"
    "{\n"
    "  fc = vec4(1.0, 0.0, 0.0, 1.0);\n"
    "}\n";

  // プログラムオブジェクトの作成
  const GLuint program(createProgram(vsrc, "pv", fsrc, "fc"));

  // 頂点属性
  static const GLfloat position[][3] =
  {
    { -0.9f,  0.9f, -0.9f },
    { -0.9f,  0.9f,  0.9f },
    {  0.9f,  0.9f, -0.9f },
    {  0.9f,  0.9f,  0.9f },
    { -0.9f, -0.9f,  0.9f },
    { -0.9f, -0.9f, -0.9f },
    {  0.9f, -0.9f, -0.9f },
    {  0.9f, -0.9f,  0.9f }
  };
  constexpr int vertices(sizeof position / sizeof position[0]);

  // 頂点インデックス
  static const GLuint index[] =
  {
    0, 1,
    0, 2,
    0, 3,
    1, 3,
    1, 4,
    2, 3,
    2, 6,
    3, 7,
    4, 5,
    4, 7,
    5, 6,
    6, 7
  };
  constexpr GLuint lines(sizeof index / sizeof index[0]);

  // 頂点配列オブジェクトの作成
  const GLuint vao(createObject(vertices, position, lines, index));

  // ウィンドウが開いている間繰り返す
  while (window)
  {
    // ウィンドウを消去する
    glClear(GL_COLOR_BUFFER_BIT);

    // シェーダプログラムの使用開始
    glUseProgram(program);

    // 描画に使う頂点配列オブジェクトの指定
    glBindVertexArray(vao);

    // 図形の描画
    glDrawElements(GL_LINES, lines, GL_UNSIGNED_INT, 0);

    // 頂点配列オブジェクトの指定解除
    glBindVertexArray(0);

    // シェーダプログラムの使用終了
    glUseProgram(0);

    // カラーバッファを入れ替えてイベントを取り出す
    window.swapBuffers();
  }
}
