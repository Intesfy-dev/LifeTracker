#ifdef USE_QT_GUI

#include "gui_window.h"
#include "io_utils.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFormLayout>
#include <QFrame>
#include <QHeaderView>
#include <QMessageBox>
#include <QTabWidget>
#include <QSplitter>
#include <QGraphicsDropShadowEffect>
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QStyle>
#include <cmath>
#include <sstream>
#include <iomanip>
#include <functional>
#include <initializer_list>
#include <cstddef>
#include <map>

namespace {

  static QString date_qstr(const date& d) {
    std::ostringstream os;
    os << d;
    return QString::fromStdString(os.str());
  }

  static void add_neumorphic_shadow(QWidget* w, int blur = 28, int dy = 8, int alpha = 190) {
    auto* eff = new QGraphicsDropShadowEffect{w};
    eff->setBlurRadius(blur);
    eff->setOffset(0, dy);
    eff->setColor(QColor{0, 0, 0, alpha});
    w->setGraphicsEffect(eff);
  }

  static QWidget* make_accent_header(const QString& text) {
    QWidget* w = new QWidget{};
    w->setStyleSheet("background: transparent;");
    QHBoxLayout* l = new QHBoxLayout{w};
    l->setContentsMargins(0, 2, 0, 4);
    l->setSpacing(10);

    QFrame* bar = new QFrame{};
    bar->setFixedSize(3, 18);
    bar->setStyleSheet("background-color: #00e65b; border-radius: 1px; border: none;");

    QLabel* lbl = new QLabel{text};
    lbl->setObjectName("SectionTitle");

    l->addWidget(bar);
    l->addWidget(lbl);
    l->addStretch();
    return w;
  }

  static int selected_row(QTableWidget* t) {
    if (!t || !t->selectionModel()) return -1;
    const auto rows = t->selectionModel()->selectedRows();
    if (!rows.isEmpty()) return rows.first().row();
    return t->currentRow();
  }

  static void setup_selectable(QTableWidget* t) {
    t->setSelectionBehavior(QAbstractItemView::SelectRows);
    t->setSelectionMode(QAbstractItemView::SingleSelection);
    t->setEditTriggers(QAbstractItemView::NoEditTriggers);
  }

  static bool confirm_action(QWidget* parent, const QString& title, const QString& text) {
    return QMessageBox::question(parent, title, text,
                                 QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes;
  }

  static void fix_hint(QWidget* parent, const QString& text) {
    QMessageBox::information(parent, "Corectie", text);
  }

  static QPushButton* make_fix_button(const QString& text, bool danger) {
    QPushButton* b = new QPushButton{text};
    b->setObjectName("SecondaryBtn");
    b->setFixedHeight(32);
    if (danger) b->setStyleSheet("color: #ff7b7b;");
    return b;
  }

  static QHBoxLayout* button_row(std::initializer_list<QPushButton*> buttons) {
    QHBoxLayout* row = new QHBoxLayout{};
    row->setSpacing(10);
    for (QPushButton* b : buttons) row->addWidget(b);
    row->addStretch();
    return row;
  }

} // namespace

// ============================================================================
// Canvas Interactiv Obsidian Graph / Game Skill Tree pentru Roadmap
// ============================================================================
class RoadmapGraphWidget : public QWidget {
public:
  struct VisNode {
    int id{0};
    int parent_id{0};
    QString title;
    double weight{1.0};
    double progress{0.0};
    double rem_effort{0.0};
    bool done{false};
    int tree_idx{0};
    QPointF pos;
    double radius{28.0};
  };

private:
  std::vector<VisNode> vnodes;
  int selected_id{-1};
  int hovered_id{-1};

  QPointF pan_offset{0.0, 0.0};
  double zoom_factor{1.0};
  bool dragging_canvas{false};
  QPoint last_mouse_pos;

  std::function<void(int)> on_select_cb;
  std::function<void(const QString&)> on_toggle_done_cb;

  QPointF world_to_screen(const QPointF& wp) const {
    QPointF center{width() / 2.0, height() / 2.0};
    return center + (wp + pan_offset) * zoom_factor;
  }

  QPointF screen_to_world(const QPointF& sp) const {
    QPointF center{width() / 2.0, height() / 2.0};
    return (sp - center) / zoom_factor - pan_offset;
  }

  int node_at_screen_pos(const QPointF& sp) const {
    const QPointF wp = screen_to_world(sp);
    for (auto it = vnodes.rbegin(); it != vnodes.rend(); ++it) {
      double dx = wp.x() - it->pos.x();
      double dy = wp.y() - it->pos.y();
      if ( std::sqrt(dx * dx + dy * dy) <= it->radius + 6.0 ) {
        return it->id;
      }
    }
    return -1;
  }

public:
  explicit RoadmapGraphWidget(QWidget* parent = nullptr) : QWidget{parent} {
    setMinimumSize(540, 380);
    setMouseTracking(true);
    setCursor(Qt::OpenHandCursor);
  }

  void set_callbacks(std::function<void(int)> sel_cb, std::function<void(const QString&)> done_cb) {
    on_select_cb = std::move(sel_cb);
    on_toggle_done_cb = std::move(done_cb);
  }

  void reset_camera() {
    pan_offset = QPointF{0.0, 0.0};
    zoom_factor = 1.0;
    update();
  }

  void set_selected_id(int id) {
    selected_id = id;
    update();
  }

  void rebuild_from_roadmap(const roadmap& rm, int current_sel_id) {
    vnodes.clear();
    selected_id = current_sel_id;

    const auto& raw = rm.get_nodes();
    if (raw.empty()) {
      update();
      return;
    }

    std::map<int, std::size_t> id_to_idx;
    for (std::size_t i = 0; i < raw.size(); ++i) {
      id_to_idx[raw[i].get_id()] = i;
      VisNode vn;
      vn.id = raw[i].get_id();
      vn.parent_id = raw[i].get_parent();
      vn.title = QString::fromStdString(raw[i].get_title());
      vn.weight = raw[i].get_weight();
      vn.done = raw[i].is_done();
      vn.progress = rm.node_progress(vn.id);
      vn.rem_effort = rm.node_remaining_effort(vn.id);
      vn.radius = (vn.parent_id == 0) ? 33.0 : 25.0;
      vnodes.push_back(vn);
    }

    auto children_of = [&](int pid) {
      std::vector<std::size_t> ch;
      for (std::size_t i = 0; i < vnodes.size(); ++i) {
        if (vnodes[i].parent_id == pid) ch.push_back(i);
      }
      return ch;
    };

    // Identificam toti arborii independenti (noduri cu parent_id == 0 sau parinte inexistent)
    std::vector<std::size_t> roots;
    for (std::size_t i = 0; i < vnodes.size(); ++i) {
      if (vnodes[i].parent_id == 0 || id_to_idx.find(vnodes[i].parent_id) == id_to_idx.end()) {
        roots.push_back(i);
      }
    }

    // Calculam latimea (in frunze) a fiecarui subarbore pentru un layout de tip Skill-Tree aerisit
    std::vector<double> leaf_span(vnodes.size(), 1.0);
    std::function<double(std::size_t)> calc_span = [&](std::size_t u) -> double {
      auto ch = children_of(vnodes[u].id);
      if (ch.empty()) {
        leaf_span[u] = 1.0;
        return 1.0;
      }
      double s = 0.0;
      for (std::size_t c : ch) s += calc_span(c);
      leaf_span[u] = std::max(1.0, s);
      return leaf_span[u];
    };

    double total_forest_span = 0.0;
    for (std::size_t r_i = 0; r_i < roots.size(); ++r_i) {
      total_forest_span += calc_span(roots[r_i]);
    }

    const double x_unit = 135.0;
    const double y_step = 120.0;
    const double tree_gap = 70.0;

    double total_width_px = total_forest_span * x_unit +
    (roots.size() > 1 ? (roots.size() - 1) * tree_gap : 0.0);
    double cursor_x = -total_width_px / 2.0;

    std::function<void(std::size_t, int, double, double, int)> place_subtree =
    [&](std::size_t u, int depth, double x_left, double x_right, int t_idx) {
      vnodes[u].tree_idx = t_idx;
      double cx = (x_left + x_right) / 2.0;
      double cy = -120.0 + depth * y_step;
      vnodes[u].pos = QPointF{cx, cy};

      auto ch = children_of(vnodes[u].id);
      if (ch.empty()) return;

      double cur = x_left;
      for (std::size_t c : ch) {
        double w_px = (leaf_span[c] / leaf_span[u]) * (x_right - x_left);
        place_subtree(c, depth + 1, cur, cur + w_px, t_idx);
        cur += w_px;
      }
    };

    for (std::size_t r_i = 0; r_i < roots.size(); ++r_i) {
      std::size_t r = roots[r_i];
      double tree_w = leaf_span[r] * x_unit;
      place_subtree(r, 0, cursor_x, cursor_x + tree_w, static_cast<int>(r_i));
      cursor_x += tree_w + tree_gap;
    }

    update();
  }

protected:
  void paintEvent(QPaintEvent*) override {
    QPainter p{this};
    p.setRenderHint(QPainter::Antialiasing, true);

    // 1. Background Obsidian Dark Canvas
    p.fillRect(rect(), QColor{"#141615"});

    // 2. Subtle Dot / Grid Matrix
    const int grid_step = std::max(18, static_cast<int>(36 * zoom_factor));
    p.setPen(QPen{QColor{46, 51, 48, 130}, 1.5});
    int ox = static_cast<int>(width() / 2.0 + pan_offset.x() * zoom_factor) % grid_step;
    int oy = static_cast<int>(height() / 2.0 + pan_offset.y() * zoom_factor) % grid_step;
    for (int x = ox; x < width(); x += grid_step) {
      for (int y = oy; y < height(); y += grid_step) {
        p.drawPoint(x, y);
      }
    }

    // Palette of subtle accent colors for multiple skill trees
    const std::vector<QColor> tree_colors = {
      QColor{"#00e65b"}, // Neon Emerald
      QColor{"#10b981"}, // Cyber Teal
      QColor{"#38bdf8"}, // Sky Neon
      QColor{"#a855f7"}, // Obsidian Purple
      QColor{"#f59e0b"}  // Amber Gold
    };

    std::map<int, std::size_t> id_map;
    for (std::size_t i = 0; i < vnodes.size(); ++i) id_map[vnodes[i].id] = i;

    // 3. Draw Connections (Curved Skill-Tree / Obsidian Links)
    for (const auto& n : vnodes) {
      if (n.parent_id != 0 && id_map.count(n.parent_id)) {
        const auto& parent = vnodes[id_map[n.parent_id]];
        QPointF p1 = world_to_screen(parent.pos);
        QPointF p2 = world_to_screen(n.pos);

        QPainterPath path;
        path.moveTo(p1);
        double mid_y = (p1.y() + p2.y()) / 2.0;
        path.cubicTo(QPointF{p1.x(), mid_y}, QPointF{p2.x(), mid_y}, p2);

        QColor base_col = tree_colors[n.tree_idx % tree_colors.size()];
        bool active_edge = (n.done || n.id == selected_id || parent.id == selected_id);

        if (active_edge) {
          QColor glow = base_col;
          glow.setAlpha(55);
          p.setPen(QPen{glow, 6.0 * zoom_factor, Qt::SolidLine, Qt::RoundCap});
          p.drawPath(path);
        }

        QColor line_col = n.done ? base_col : QColor{"#3d4440"};
        if (n.id == selected_id || parent.id == selected_id) line_col = base_col.lighter(120);
        p.setPen(QPen{line_col, 2.2 * zoom_factor, n.done ? Qt::SolidLine : Qt::DashLine});
        p.drawPath(path);
      }
    }

    // 4. Draw Circular Nodes
    for (const auto& n : vnodes) {
      QPointF sc = world_to_screen(n.pos);
      double r = n.radius * zoom_factor;
      QColor tcol = tree_colors[n.tree_idx % tree_colors.size()];
      bool is_sel = (n.id == selected_id);
      bool is_hov = (n.id == hovered_id);

      // Outer Glow for Root, Selected, or Hovered Node
      if (is_sel || is_hov || n.parent_id == 0) {
        QColor glow = is_sel ? QColor{"#00e65b"} : tcol;
        glow.setAlpha(is_sel ? 90 : (is_hov ? 65 : 35));
        p.setPen(Qt::NoPen);
        p.setBrush(glow);
        p.drawEllipse(sc, r + 8.0 * zoom_factor, r + 8.0 * zoom_factor);
      }

      // Main Node Circle Body
      QColor fill_col = n.done ? QColor{"#132a1d"} : QColor{"#1f2321"};
      if (is_sel) fill_col = QColor{"#193826"};
      p.setBrush(fill_col);
      p.setPen(QPen{QColor{"#2e3532"}, 2.0 * zoom_factor});
      p.drawEllipse(sc, r, r);

      // Progress Ring Arc around Node
      QRectF arc_rect{sc.x() - r, sc.y() - r, r * 2.0, r * 2.0};
      int span_angle = static_cast<int>(-(n.progress / 100.0) * 360.0 * 16.0);
      QColor ring_col = n.done ? QColor{"#00e65b"} : (is_sel ? QColor{"#ffffff"} : tcol);
      p.setPen(QPen{ring_col, (is_sel ? 3.4 : 2.4) * zoom_factor, Qt::SolidLine, Qt::RoundCap});
      p.setBrush(Qt::NoBrush);
      if (n.progress >= 99.9) {
        p.drawEllipse(sc, r, r);
      } else if (n.progress > 0.5) {
        p.drawArc(arc_rect, 90 * 16, span_angle);
      }

      // Root Crown / Star Indicator or Percentage Inside Node
      QFont f = font();
      f.setPointSizeF(std::max(7.5, (n.parent_id == 0 ? 10.0 : 9.0) * zoom_factor));
      f.setBold(true);
      p.setFont(f);
      p.setPen(n.done ? QColor{"#00e65b"} : QColor{"#f4f4f6"});

      QString center_txt = n.done ? "✓" : QString("%1%").arg(static_cast<int>(n.progress));
      p.drawText(arc_rect, Qt::AlignCenter, center_txt);

      // Root Badge Label ("TREE ROOT")
      if (n.parent_id == 0) {
        QFont rf = font();
        rf.setPointSizeF(std::max(6.5, 7.5 * zoom_factor));
        rf.setBold(true);
        p.setFont(rf);
        p.setPen(tcol);
        QRectF badge_rect{sc.x() - 70 * zoom_factor, sc.y() - r - 20 * zoom_factor, 140 * zoom_factor, 16 * zoom_factor};
        p.drawText(badge_rect, Qt::AlignCenter, QString("★ ARBORE #%1").arg(n.tree_idx + 1));
      }

      // Node Title Pill Underneath Circle
      QFont tf = font();
      tf.setPointSizeF(std::max(7.5, 9.2 * zoom_factor));
      tf.setBold(is_sel || n.parent_id == 0);
      p.setFont(tf);

      QRectF lbl_rect{sc.x() - 95 * zoom_factor, sc.y() + r + 5 * zoom_factor, 190 * zoom_factor, 34 * zoom_factor};
      p.setPen(is_sel ? QColor{"#00e65b"} : QColor{"#e4e4e7"});
      p.drawText(lbl_rect, Qt::AlignHCenter | Qt::AlignTop | Qt::TextWordWrap, n.title);
    }

    // 5. Top-left overlay legend
    p.setPen(QColor{"#8e9290"});
    QFont hf = font();
    hf.setPointSize(9);
    p.setFont(hf);
    p.drawText(14, 22, "Click pe cerc = inspecteaza sub-task-uri & selecteaza parinte   |   Dublu-click = finalizeaza   |   Drag & Scroll = Pan / Zoom");
  }

  void mousePressEvent(QMouseEvent* ev) override {
    if (ev->button() == Qt::LeftButton) {
      int hit = node_at_screen_pos(ev->pos());
      if (hit != -1) {
        selected_id = hit;
        if (on_select_cb) on_select_cb(hit);
        update();
        return;
      }
      dragging_canvas = true;
      last_mouse_pos = ev->pos();
      setCursor(Qt::ClosedHandCursor);
    }
  }

  void mouseMoveEvent(QMouseEvent* ev) override {
    if (dragging_canvas) {
      QPoint delta = ev->pos() - last_mouse_pos;
      last_mouse_pos = ev->pos();
      pan_offset += QPointF{delta.x() / zoom_factor, delta.y() / zoom_factor};
      update();
      return;
    }
    int hit = node_at_screen_pos(ev->pos());
    if (hit != hovered_id) {
      hovered_id = hit;
      setCursor(hit != -1 ? Qt::PointingHandCursor : Qt::OpenHandCursor);
      update();
    }
  }

  void mouseReleaseEvent(QMouseEvent*) override {
    dragging_canvas = false;
    setCursor(hovered_id != -1 ? Qt::PointingHandCursor : Qt::OpenHandCursor);
  }

  void mouseDoubleClickEvent(QMouseEvent* ev) override {
    int hit = node_at_screen_pos(ev->pos());
    if (hit != -1 && on_toggle_done_cb) {
      for (const auto& n : vnodes) {
        if (n.id == hit) {
          on_toggle_done_cb(n.title);
          break;
        }
      }
    }
  }

  void wheelEvent(QWheelEvent* ev) override {
    double factor = (ev->angleDelta().y() > 0) ? 1.12 : 0.89;
    zoom_factor = std::clamp(zoom_factor * factor, 0.45, 2.25);
    update();
  }
};

// ============================================================================
// Fereastra Principala GUI
// ============================================================================
gui_window::gui_window(account_manager& m, QWidget* parent)
: QMainWindow{parent}, mgr{m} {
  setWindowTitle("Life Tracker OS — Matte Black & Neon Edition");
  resize(1340, 850);
  apply_emerald_theme();

  root_stack = new QStackedWidget{this};
  root_stack->addWidget(build_auth_page());
  root_stack->addWidget(build_main_app_page());
  setCentralWidget(root_stack);
  root_stack->setCurrentIndex(0);
}

void gui_window::apply_emerald_theme() {
  setStyleSheet(R"(
    QMainWindow, QWidget {
      background-color: #1c1e1d;
      color: #f4f4f6;
      font-family: 'Inter', 'Segoe UI', 'Noto Sans', sans-serif;
      font-size: 13px;
    }
    QFrame#Card, QFrame#AuthCard, QFrame#Sidebar, QFrame#TopBar {
      background-color: #232524;
      border: 1px solid #2e3130;
      border-radius: 16px;
    }
    QFrame#StatCard {
      background-color: #232524;
      border: 1px solid #2e3130;
      border-radius: 16px;
    }
    QLabel#HeroTitle {
      font-size: 24px;
      font-weight: 800;
      letter-spacing: 1px;
      color: #ffffff;
      background: transparent;
    }
    QLabel#SectionTitle {
      font-size: 15px;
      font-weight: 700;
      color: #ffffff;
      background: transparent;
    }
    QLabel#StatValue {
      font-size: 25px;
      font-weight: 800;
      color: #ffffff;
      background: transparent;
    }
    QLabel#StatValueNeon {
      font-size: 25px;
      font-weight: 800;
      color: #00e65b;
      background: transparent;
    }
    QLabel#StatLabel {
      font-size: 12px;
      font-weight: 700;
      color: #ffffff;
      background: transparent;
    }
    QLabel#StatSubLabel {
      font-size: 11px;
      color: #8e9290;
      background: transparent;
    }
    QLabel {
      background: transparent;
      color: #d4d7d5;
    }
    QLineEdit, QSpinBox, QDoubleSpinBox, QComboBox, QTextEdit, QListWidget {
      background-color: #171918;
      border: 1px solid #2e3130;
      border-radius: 9px;
      padding: 7px 12px;
      color: #ffffff;
      selection-background-color: #00e65b;
      selection-color: #08120c;
    }
    QLineEdit:focus, QSpinBox:focus, QDoubleSpinBox:focus, QComboBox:focus, QTextEdit:focus {
      border: 1.5px solid #00e65b;
      background-color: #141615;
    }
    QComboBox::drop-down {
      border: none;
      width: 24px;
    }
    QPushButton {
      background-color: #00e65b;
      color: #08120c;
      font-weight: 800;
      border: 1.5px solid #a7f3d0;
      border-radius: 10px;
      padding: 9px 18px;
    }
    QPushButton:hover {
      background-color: #1ff06e;
      border: 1.5px solid #ffffff;
    }
    QPushButton:pressed {
      background-color: #00c44d;
    }
    QPushButton#NavBtn {
      background-color: #1a1c1b;
      color: #9ca3af;
      text-align: left;
      padding: 11px 16px;
      border: 1px solid #282b29;
      border-radius: 10px;
      font-size: 13px;
      font-weight: 600;
    }
    QPushButton#NavBtn:hover {
      background-color: #262928;
      color: #ffffff;
      border: 1px solid #3a3f3c;
    }
    QPushButton#NavBtnActive {
      background-color: #00e65b;
      color: #07120b;
      text-align: left;
      padding: 11px 16px;
      border: 1.5px solid #d1fae5;
      border-radius: 10px;
      font-size: 13px;
      font-weight: 800;
    }
    QPushButton#SecondaryBtn {
      background-color: #191b1a;
      color: #e4e4e7;
      border: 1px solid #323634;
      border-radius: 10px;
      font-weight: 700;
    }
    QPushButton#SecondaryBtn:hover {
      background-color: #252827;
      border: 1px solid #00e65b;
      color: #00e65b;
    }
    QTableWidget {
      background-color: #181a19;
      alternate-background-color: #1e201f;
      gridline-color: #2a2d2b;
      border: 1px solid #2e3130;
      border-radius: 10px;
      color: #f4f4f6;
    }
    QHeaderView::section {
      background-color: #202321;
      color: #00e65b;
      font-weight: 700;
      padding: 7px;
      border: none;
      border-bottom: 1.5px solid #2e3130;
    }
    QProgressBar {
      background-color: #161817;
      border: 1px solid #2e3130;
      border-radius: 6px;
      text-align: center;
      color: #ffffff;
      font-weight: bold;
    }
    QProgressBar::chunk {
      background-color: #00e65b;
      border-radius: 5px;
    }
    QProgressBar#ThinVertBar {
      background-color: #151716;
      border: none;
      border-radius: 2px;
    }
    QProgressBar#ThinVertBar::chunk {
      background-color: #00e65b;
      border-radius: 2px;
    }
    QTabWidget::pane {
      border: 1px solid #2e3130;
      border-radius: 12px;
      background-color: #232524;
    }
    QTabBar::tab {
      background-color: #181a19;
      color: #9ca3af;
      padding: 10px 22px;
      border-top-left-radius: 9px;
      border-top-right-radius: 9px;
      margin-right: 6px;
      border: 1px solid #2a2d2b;
      font-weight: 600;
    }
    QTabBar::tab:selected {
      background-color: #00e65b;
      color: #07120b;
      font-weight: 800;
      border: 1px solid #a7f3d0;
    }
    QCheckBox {
      color: #e4e4e7;
      spacing: 10px;
      font-weight: 600;
      background: transparent;
    }
    QCheckBox::indicator {
      width: 40px;
      height: 20px;
      border-radius: 10px;
      background-color: #161817;
      border: 1.5px solid #00e65b;
    }
    QCheckBox::indicator:checked {
      background-color: #00e65b;
      border: 1.5px solid #a7f3d0;
    }
  )");
}

// ============================================================================
// Ecranul Principal de Login — Cu descrierea curata a proiectului in stanga
// ============================================================================
QWidget* gui_window::build_auth_page() {
  QWidget* page = new QWidget{};
  QHBoxLayout* outer = new QHBoxLayout{page};
  outer->setContentsMargins(55, 45, 55, 45);
  outer->setSpacing(38);

  // Left Project Description Panel (fara butoanele inutile)
  QFrame* brand_card = new QFrame{};
  brand_card->setObjectName("AuthCard");
  add_neumorphic_shadow(brand_card, 35, 10, 210);
  QVBoxLayout* brand_layout = new QVBoxLayout{brand_card};
  brand_layout->setContentsMargins(38, 36, 38, 36);
  brand_layout->setSpacing(16);

  brand_layout->addWidget(make_accent_header("DESCRIEREA PROIECTULUI"));

  QLabel* hero_title = new QLabel{"LIFE TRACKER OS"};
  hero_title->setObjectName("HeroTitle");
  brand_layout->addWidget(hero_title);

  QLabel* proj_desc = new QLabel{
    "Aplicatie C++ orientata pe obiecte pentru gestionarea integrata a vietii personale, "
    "proiectelor tehnice si obiectivelor pe termen lung intr-o singura interfata unificata.\n\n"
    "Arhitectura modulara include:"
  };
  proj_desc->setWordWrap(true);
  proj_desc->setStyleSheet("color: #d4d7d5; font-size: 13.5px; line-height: 1.45;");
  brand_layout->addWidget(proj_desc);

  auto make_feature_bullet = [](const QString& title, const QString& desc) {
    QFrame* row = new QFrame{};
    row->setStyleSheet("background-color: #1b1d1c; border: 1px solid #2b2e2d; border-radius: 10px;");
    QHBoxLayout* hl = new QHBoxLayout{row};
    hl->setContentsMargins(14, 10, 14, 10);
    hl->setSpacing(12);

    QLabel* dot = new QLabel{"◈"};
    dot->setStyleSheet("color: #00e65b; font-size: 15px; font-weight: bold;");

    QLabel* txt = new QLabel{QString("<b>%1</b> — <span style='color:#9ca3af;'>%2</span>").arg(title, desc)};
    txt->setWordWrap(true);

    hl->addWidget(dot);
    hl->addWidget(txt, 1);
    return row;
  };

  brand_layout->addWidget(make_feature_bullet(
    "Buget & Obiective Financiare",
    "Evidenta veniturilor, cheltuielilor, abonamentelor recurente si prognoza soldului."
  ));
  brand_layout->addWidget(make_feature_bullet(
    "Smart Tasks & Cautare Binara",
    "Prioritizare interactiva prin comparatii binare si scoruri dinamice de urgenta."
  ));
  brand_layout->addWidget(make_feature_bullet(
    "Roadmap Vizual (Skill-Tree & Obsidian Graph)",
                                              "Graf interactiv cu mai multi arbori independenti, noduri circulare si inspectie sub-task-uri."
  ));
  brand_layout->addWidget(make_feature_bullet(
    "Calendar (.ICS Sync) & Habits/Fitness",
                                              "Detectie automata a conflictelor orare, export Google Calendar si monitorizare antrenamente."
  ));

  brand_layout->addStretch();

  // Demo account hint card at bottom
  QFrame* msg_box = new QFrame{};
  msg_box->setStyleSheet("background-color: #1b1d1c; border: 1px solid #2e3130; border-radius: 12px;");
  QHBoxLayout* msg_lay = new QHBoxLayout{msg_box};
  msg_lay->setContentsMargins(16, 14, 16, 14);
  msg_lay->setSpacing(12);

  QFrame* green_bar = new QFrame{};
  green_bar->setFixedWidth(3);
  green_bar->setStyleSheet("background-color: #00e65b; border-radius: 1px;");

  QLabel* demo_hint = new QLabel{
    "<b>Cont preconfigurat (din data/user.txt):</b><br>"
    "Utilizator: <span style='color:#00e65b; font-weight:bold;'>Arian</span> &nbsp;|&nbsp; "
    "Parola: <span style='color:#00e65b; font-weight:bold;'>Cyberpunk</span><br>"
    "<span style='color:#8e9290;'>Datele fiecarui cont sunt salvate automat in data/users/&lt;user&gt;/.</span>"
  };
  demo_hint->setWordWrap(true);

  msg_lay->addWidget(green_bar);
  msg_lay->addWidget(demo_hint, 1);
  brand_layout->addWidget(msg_box);

  // Right Auth Tab Card
  QFrame* form_card = new QFrame{};
  form_card->setObjectName("AuthCard");
  add_neumorphic_shadow(form_card, 35, 10, 210);
  form_card->setMinimumWidth(450);
  QVBoxLayout* form_layout = new QVBoxLayout{form_card};
  form_layout->setContentsMargins(32, 32, 32, 32);

  QTabWidget* auth_tabs = new QTabWidget{};

  // Tab 1: Login
  QWidget* login_tab = new QWidget{};
  QVBoxLayout* l_lay = new QVBoxLayout{login_tab};
  l_lay->setContentsMargins(22, 26, 22, 22);
  l_lay->setSpacing(14);

  l_lay->addWidget(make_accent_header("Autentificare in Cont"));
  login_user_edit = new QLineEdit{"Arian"};
  login_user_edit->setPlaceholderText("Nume utilizator...");
  login_pass_edit = new QLineEdit{"Cyberpunk"};
  login_pass_edit->setEchoMode(QLineEdit::Password);
  login_pass_edit->setPlaceholderText("Parola...");

  QPushButton* login_btn = new QPushButton{"⏻   PORNESTE SISTEMUL (LOGIN)"};
  login_btn->setMinimumHeight(44);
  connect(login_btn, &QPushButton::clicked, this, &gui_window::handle_login);

  l_lay->addWidget(new QLabel{"UTILIZATOR"});
  l_lay->addWidget(login_user_edit);
  l_lay->addWidget(new QLabel{"PAROLA"});
  l_lay->addWidget(login_pass_edit);
  l_lay->addSpacing(12);
  l_lay->addWidget(login_btn);
  l_lay->addStretch();

  // Tab 2: Sign Up
  QWidget* reg_tab = new QWidget{};
  QVBoxLayout* r_lay = new QVBoxLayout{reg_tab};
  r_lay->setContentsMargins(22, 20, 22, 20);
  r_lay->setSpacing(10);

  r_lay->addWidget(make_accent_header("Inregistrare Cont Nou"));
  reg_user_edit = new QLineEdit{};
  reg_user_edit->setPlaceholderText("Ex: arian_dev");
  reg_pass_edit = new QLineEdit{};
  reg_pass_edit->setEchoMode(QLineEdit::Password);
  reg_pass_edit->setPlaceholderText("Alege o parola...");
  reg_date_edit = new QLineEdit{"17.11.2026"};
  reg_balance_spin = new QDoubleSpinBox{};
  reg_balance_spin->setRange(0, 1000000);
  reg_balance_spin->setValue(3500);
  reg_balance_spin->setSuffix(" RON");
  reg_seed_check = new QCheckBox{"ON  —  Populeaza cu date demo initiale"};
  reg_seed_check->setChecked(true);

  QPushButton* reg_btn = new QPushButton{"＋   CREEAZA CONT & INTRA"};
  reg_btn->setMinimumHeight(42);
  connect(reg_btn, &QPushButton::clicked, this, &gui_window::handle_register);

  r_lay->addWidget(new QLabel{"UTILIZATOR NOU"});
  r_lay->addWidget(reg_user_edit);
  r_lay->addWidget(new QLabel{"PAROLA"});
  r_lay->addWidget(reg_pass_edit);
  r_lay->addWidget(new QLabel{"DATA CURENTA (Z.L.A)"});
  r_lay->addWidget(reg_date_edit);
  r_lay->addWidget(new QLabel{"SOLD INITIAL BUGET"});
  r_lay->addWidget(reg_balance_spin);
  r_lay->addSpacing(4);
  r_lay->addWidget(reg_seed_check);
  r_lay->addSpacing(8);
  r_lay->addWidget(reg_btn);
  r_lay->addStretch();

  auth_tabs->addTab(login_tab, "⏻  LOGIN");
  auth_tabs->addTab(reg_tab, "＋  CONT NOU");

  auth_status_lbl = new QLabel{""};
  auth_status_lbl->setStyleSheet("color: #ff4d4f; font-weight: bold;");

  form_layout->addWidget(auth_tabs);
  form_layout->addWidget(auth_status_lbl);

  outer->addWidget(brand_card, 3);
  outer->addWidget(form_card, 2);
  return page;
}

QWidget* gui_window::build_main_app_page() {
  QWidget* page = new QWidget{};
  QHBoxLayout* main_h = new QHBoxLayout{page};
  main_h->setContentsMargins(16, 16, 16, 16);
  main_h->setSpacing(16);

  // Left Neumorphic Dark Sidebar
  QFrame* sidebar = new QFrame{};
  sidebar->setObjectName("Sidebar");
  add_neumorphic_shadow(sidebar, 26, 6, 180);
  sidebar->setFixedWidth(245);
  QVBoxLayout* side_lay = new QVBoxLayout{sidebar};
  side_lay->setContentsMargins(16, 22, 16, 22);
  side_lay->setSpacing(9);

  side_lay->addWidget(make_accent_header("LIFE TRACKER"));
  side_lay->addSpacing(8);

  const std::vector<std::string> labels = {
    "◈   Dashboard",
    "◈   Buget & Finante",
    "◈   Smart Tasks",
    "◈   Calendar & Sync",
    "◈   Roadmap Skill-Tree",
    "◈   Habits & Fitness"
  };

  for (std::size_t i = 0; i < labels.size(); ++i) {
    QPushButton* b = new QPushButton{QString::fromStdString(labels[i])};
    b->setObjectName(i == 0 ? "NavBtnActive" : "NavBtn");
    int idx = static_cast<int>(i);
    connect(b, &QPushButton::clicked, this, [this, idx]() { switch_nav_tab(idx); });
    nav_buttons.push_back(b);
    side_lay->addWidget(b);
  }

  side_lay->addStretch();

  QPushButton* next_day_btn = new QPushButton{"»   Avanseaza Ziua (+1)"};
  next_day_btn->setObjectName("SecondaryBtn");
  connect(next_day_btn, &QPushButton::clicked, this, &gui_window::handle_next_day);
  side_lay->addWidget(next_day_btn);

  QPushButton* logout_btn = new QPushButton{"⏻   Salveaza & Iesi"};
  logout_btn->setObjectName("SecondaryBtn");
  connect(logout_btn, &QPushButton::clicked, this, &gui_window::handle_logout);
  side_lay->addWidget(logout_btn);

  // Right Content Area (Topbar + Stacked Pages)
  QVBoxLayout* right_v = new QVBoxLayout{};
  right_v->setSpacing(14);

  QFrame* topbar = new QFrame{};
  topbar->setObjectName("TopBar");
  add_neumorphic_shadow(topbar, 22, 5, 170);
  topbar->setFixedHeight(68);
  QHBoxLayout* top_h = new QHBoxLayout{topbar};
  top_h->setContentsMargins(22, 10, 22, 10);

  QFrame* top_bar_accent = new QFrame{};
  top_bar_accent->setFixedSize(3, 22);
  top_bar_accent->setStyleSheet("background-color: #00e65b; border-radius: 1px;");

  header_user_lbl = new QLabel{"CONT: -"};
  header_user_lbl->setStyleSheet("font-size: 14px; font-weight: 800; color: #ffffff;");
  header_date_lbl = new QLabel{"DATA: -"};
  header_date_lbl->setStyleSheet("font-size: 13px; font-weight: 600; color: #9ca3af;");
  header_score_lbl = new QLabel{"LIFE SCORE"};
  header_score_lbl->setStyleSheet("font-size: 12px; font-weight: 800; color: #00e65b; letter-spacing: 1px;");

  header_score_bar = new QProgressBar{};
  header_score_bar->setRange(0, 100);
  header_score_bar->setFixedWidth(190);
  header_score_bar->setFixedHeight(22);

  top_h->addWidget(top_bar_accent);
  top_h->addSpacing(8);
  top_h->addWidget(header_user_lbl);
  top_h->addSpacing(28);
  top_h->addWidget(header_date_lbl);
  top_h->addStretch();
  top_h->addWidget(header_score_lbl);
  top_h->addSpacing(8);
  top_h->addWidget(header_score_bar);

  module_stack = new QStackedWidget{};
  module_stack->addWidget(build_dashboard_tab());
  module_stack->addWidget(build_budget_tab());
  module_stack->addWidget(build_tasks_tab());
  module_stack->addWidget(build_calendar_tab());
  module_stack->addWidget(build_roadmap_tab());
  module_stack->addWidget(build_habits_fitness_tab());

  right_v->addWidget(topbar);
  right_v->addWidget(module_stack, 1);

  main_h->addWidget(sidebar);
  main_h->addLayout(right_v, 1);
  return page;
}

QWidget* gui_window::build_dashboard_tab() {
  QWidget* w = new QWidget{};
  QVBoxLayout* v = new QVBoxLayout{w};
  v->setContentsMargins(0, 0, 0, 0);
  v->setSpacing(14);

  QHBoxLayout* cards_h = new QHBoxLayout{};
  cards_h->setSpacing(14);
  card_vbars.clear();

  auto make_stat = [&](const QString& title, const QString& subtitle, QLabel*& val_lbl, bool neon_text) {
    QFrame* c = new QFrame{};
    c->setObjectName("StatCard");
    add_neumorphic_shadow(c, 24, 6, 180);
    c->setMinimumHeight(128);

    QHBoxLayout* ch = new QHBoxLayout{c};
    ch->setContentsMargins(18, 16, 18, 16);
    ch->setSpacing(12);

    QVBoxLayout* cl = new QVBoxLayout{};
    cl->setSpacing(2);
    QLabel* t = new QLabel{title};
    t->setObjectName("StatLabel");
    QLabel* sub = new QLabel{subtitle};
    sub->setObjectName("StatSubLabel");

    val_lbl = new QLabel{"0%"};
    val_lbl->setObjectName(neon_text ? "StatValueNeon" : "StatValue");

    cl->addWidget(t);
    cl->addWidget(sub);
    cl->addStretch();
    cl->addWidget(val_lbl);

    QProgressBar* vbar = new QProgressBar{};
    vbar->setObjectName("ThinVertBar");
    vbar->setOrientation(Qt::Vertical);
    vbar->setRange(0, 100);
    vbar->setValue(50);
    vbar->setTextVisible(false);
    vbar->setFixedWidth(4);

    card_vbars.push_back(vbar);

    ch->addLayout(cl, 1);
    ch->addWidget(vbar);
    cards_h->addWidget(c);
  };

  make_stat("Sold Buget", "Disponibil curent", card_budget_val, true);
  make_stat("Smart Tasks", "Ritm finalizare", card_tasks_val, false);
  make_stat("Roadmap", "Progres arbori", card_roadmap_val, true);
  make_stat("Habits", "Consistenta zilnica", card_habits_val, false);
  make_stat("Fitness", "Tinta kcal saptamanala", card_fitness_val, true);

  v->addLayout(cards_h);

  QFrame* rep_card = new QFrame{};
  rep_card->setObjectName("Card");
  add_neumorphic_shadow(rep_card, 26, 7, 190);
  QVBoxLayout* rl = new QVBoxLayout{rep_card};
  rl->setContentsMargins(20, 18, 20, 18);
  rl->setSpacing(10);

  rl->addWidget(make_accent_header("Raport Executiv Live  —  lifetracker::summary()"));
  dashboard_console = new QTextEdit{};
  dashboard_console->setReadOnly(true);
  dashboard_console->setStyleSheet(
    "font-family: 'JetBrains Mono', 'Fira Code', 'Consolas', monospace; "
    "font-size: 13px; background-color: #161817; color: #d4d7d5; border: 1px solid #282b29; border-radius: 10px; padding: 10px;"
  );

  rl->addWidget(dashboard_console, 1);
  v->addWidget(rep_card, 1);

  return w;
}

QWidget* gui_window::build_budget_tab() {
  QWidget* w = new QWidget{};
  QHBoxLayout* h = new QHBoxLayout{w};
  h->setContentsMargins(0, 0, 0, 0);
  h->setSpacing(14);

  QVBoxLayout* left_v = new QVBoxLayout{};
  QFrame* tbl_card = new QFrame{};
  tbl_card->setObjectName("Card");
  add_neumorphic_shadow(tbl_card, 24, 6, 180);
  QVBoxLayout* tl = new QVBoxLayout{tbl_card};
  tl->setContentsMargins(18, 16, 18, 16);

  tl->addWidget(make_accent_header("Istoric Tranzactii & Abonamente"));
  tx_table = new QTableWidget{0, 6};
  tx_table->setHorizontalHeaderLabels({"Titlu", "Categorie", "Data", "Suma (RON)", "Abonament", "Win"});
  tx_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  tx_table->setAlternatingRowColors(true);

  setup_selectable(tx_table);
  QPushButton* del_tx_btn = make_fix_button("🗑  STERGE TRANZACTIA SELECTATA", true);
  connect(del_tx_btn, &QPushButton::clicked, this, [this]() {
    int row = selected_row(tx_table);
    const auto& txs = current_app.budget_ref().get_transactions();
    if (row < 0 || row >= static_cast<int>(txs.size())) {
      fix_hint(this, "Selecteaza intai o tranzactie din tabel.");
      return;
    }
    QString name = QString::fromStdString(txs[static_cast<std::size_t>(row)].get_title());
    if (!confirm_action(this, "Sterge tranzactia", QString("Stergi tranzactia \"%1\"? Soldul se recalculeaza automat.").arg(name))) return;
    if (current_app.budget_ref().remove_transaction(static_cast<std::size_t>(row))) {
      mgr.save_user_app(current_app, current_initial_balance);
      refresh_all();
    }
  });

  tl->addWidget(tx_table, 2);
  tl->addLayout(button_row({del_tx_btn}));
  tl->addWidget(make_accent_header("Obiective Financiare (Money Goals)"));

  goals_table = new QTableWidget{0, 4};
  goals_table->setHorizontalHeaderLabels({"Obiectiv", "Suma Tinta", "Deadline", "Zile Ramase"});
  goals_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  goals_table->setMaximumHeight(155);

  budget_report_box = new QTextEdit{};
  budget_report_box->setReadOnly(true);
  budget_report_box->setMaximumHeight(150);
  budget_report_box->setStyleSheet("font-family: 'JetBrains Mono', monospace; font-size: 12px;");

  setup_selectable(goals_table);
  QPushButton* del_goal_btn = make_fix_button("🗑  STERGE OBIECTIVUL SELECTAT", true);
  connect(del_goal_btn, &QPushButton::clicked, this, [this]() {
    int row = selected_row(goals_table);
    const auto& goals = current_app.budget_ref().get_goals();
    if (row < 0 || row >= static_cast<int>(goals.size())) {
      fix_hint(this, "Selecteaza intai un obiectiv din tabel.");
      return;
    }
    QString name = QString::fromStdString(goals[static_cast<std::size_t>(row)].get_title());
    if (!confirm_action(this, "Sterge obiectivul", QString("Stergi obiectivul \"%1\"?").arg(name))) return;
    if (current_app.budget_ref().remove_goal(static_cast<std::size_t>(row))) {
      mgr.save_user_app(current_app, current_initial_balance);
      refresh_all();
    }
  });

  tl->addWidget(goals_table, 1);
  tl->addLayout(button_row({del_goal_btn}));
  tl->addWidget(budget_report_box, 1);
  left_v->addWidget(tbl_card);

  QFrame* form_card = new QFrame{};
  form_card->setObjectName("Card");
  add_neumorphic_shadow(form_card, 24, 6, 180);
  form_card->setFixedWidth(345);
  QVBoxLayout* fl = new QVBoxLayout{form_card};
  fl->setContentsMargins(18, 16, 18, 16);

  fl->addWidget(make_accent_header("Adauga Tranzactie"));
  tx_title_edit = new QLineEdit{};
  tx_title_edit->setPlaceholderText("Ex: Cumparaturi / Bursa...");
  tx_cat_combo = new QComboBox{};
  tx_cat_combo->addItems({"food", "transportation", "shopping", "bills", "housing", "health", "entertainment", "education", "other", "salary", "bonus"});
  tx_date_edit = new QLineEdit{"17.11.2026"};
  tx_amount_spin = new QDoubleSpinBox{};
  tx_amount_spin->setRange(1, 1000000);
  tx_amount_spin->setValue(150);
  tx_sub_check = new QCheckBox{"Abonament lunar (ON/OFF)"};
  tx_win_check = new QCheckBox{"Castig / Bonus (ON/OFF)"};

  QPushButton* add_tx_btn = new QPushButton{"＋  ADAUGA TRANZACTIE"};
  connect(add_tx_btn, &QPushButton::clicked, this, [this]() {
    std::ostringstream ss;
    ss << tx_title_edit->text().toStdString() << ","
    << tx_cat_combo->currentText().toStdString() << ","
    << tx_date_edit->text().toStdString() << ","
    << tx_amount_spin->value() << ","
    << (tx_sub_check->isChecked() ? 1 : 0) << ","
    << (tx_win_check->isChecked() ? 1 : 0);
    transaction t;
    if (!tx_title_edit->text().isEmpty() && parse_line(ss.str(), t)) {
      current_app.add_transaction(t);
      mgr.save_user_app(current_app, current_initial_balance);
      tx_title_edit->clear();
      refresh_all();
    }
  });

  goal_title_edit = new QLineEdit{};
  goal_title_edit->setPlaceholderText("Ex: Monitor OLED...");
  goal_amount_spin = new QDoubleSpinBox{};
  goal_amount_spin->setRange(10, 1000000);
  goal_amount_spin->setValue(1500);
  goal_date_edit = new QLineEdit{"01.05.2027"};

  QPushButton* add_goal_btn = new QPushButton{"＋  ADAUGA OBIECTIV"};
  add_goal_btn->setObjectName("SecondaryBtn");
  connect(add_goal_btn, &QPushButton::clicked, this, [this]() {
    date d{1, 5, 2027};
    parse_line(goal_date_edit->text().toStdString(), d);
    if (!goal_title_edit->text().isEmpty()) {
      current_app.budget_ref().add_goal(money_goal{goal_title_edit->text().toStdString(), goal_amount_spin->value(), d});
      mgr.save_user_app(current_app, current_initial_balance);
      goal_title_edit->clear();
      refresh_all();
    }
  });

  fl->addWidget(new QLabel{"Titlu:"});
  fl->addWidget(tx_title_edit);
  fl->addWidget(new QLabel{"Categorie:"});
  fl->addWidget(tx_cat_combo);
  fl->addWidget(new QLabel{"Data (z.l.a):"});
  fl->addWidget(tx_date_edit);
  fl->addWidget(new QLabel{"Suma (RON):"});
  fl->addWidget(tx_amount_spin);
  fl->addWidget(tx_sub_check);
  fl->addWidget(tx_win_check);
  fl->addWidget(add_tx_btn);
  fl->addSpacing(14);
  fl->addWidget(make_accent_header("Obiectiv Financiar Nou"));
  fl->addWidget(new QLabel{"Titlu Obiectiv:"});
  fl->addWidget(goal_title_edit);
  fl->addWidget(new QLabel{"Suma Tinta (RON):"});
  fl->addWidget(goal_amount_spin);
  fl->addWidget(new QLabel{"Deadline (z.l.a):"});
  fl->addWidget(goal_date_edit);
  fl->addWidget(add_goal_btn);
  fl->addStretch();

  h->addLayout(left_v, 1);
  h->addWidget(form_card);
  return w;
}

QWidget* gui_window::build_tasks_tab() {
  QWidget* w = new QWidget{};
  QHBoxLayout* h = new QHBoxLayout{w};
  h->setContentsMargins(0, 0, 0, 0);
  h->setSpacing(14);

  QFrame* left_card = new QFrame{};
  left_card->setObjectName("Card");
  add_neumorphic_shadow(left_card, 24, 6, 180);
  QVBoxLayout* ll = new QVBoxLayout{left_card};
  ll->setContentsMargins(18, 16, 18, 16);

  ll->addWidget(make_accent_header("Lista Sarcini & Prioritati"));
  tasks_table = new QTableWidget{0, 6};
  tasks_table->setHorizontalHeaderLabels({"Titlu", "Descriere", "Urgenta (1-3)", "Deadline", "Progres", "Status"});
  tasks_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  tasks_table->setAlternatingRowColors(true);

  tasks_focus_box = new QTextEdit{};
  tasks_focus_box->setReadOnly(true);
  tasks_focus_box->setMaximumHeight(160);
  tasks_focus_box->setStyleSheet("font-family: 'JetBrains Mono', monospace;");

  setup_selectable(tasks_table);
  QPushButton* reopen_task_btn = make_fix_button("↩  REDESCHIDE (anuleaza Finalizat)", false);
  QPushButton* del_task_btn = make_fix_button("🗑  STERGE TASKUL SELECTAT", true);
  connect(reopen_task_btn, &QPushButton::clicked, this, [this]() {
    const auto sorted = current_app.tasks_ref().prioritize();
    int row = selected_row(tasks_table);
    if (row < 0 || row >= static_cast<int>(sorted.size())) {
      fix_hint(this, "Selecteaza intai un task din tabel.");
      return;
    }
    const std::string title = sorted[static_cast<std::size_t>(row)].get_title();
    if (!sorted[static_cast<std::size_t>(row)].is_done()) {
      fix_hint(this, "Taskul selectat nu este marcat ca finalizat.");
      return;
    }
    if (current_app.tasks_ref().reopen(title, current_app.get_date())) {
      mgr.save_user_app(current_app, current_initial_balance);
      refresh_all();
    }
  });
  connect(del_task_btn, &QPushButton::clicked, this, [this]() {
    const auto sorted = current_app.tasks_ref().prioritize();
    int row = selected_row(tasks_table);
    if (row < 0 || row >= static_cast<int>(sorted.size())) {
      fix_hint(this, "Selecteaza intai un task din tabel.");
      return;
    }
    const std::string title = sorted[static_cast<std::size_t>(row)].get_title();
    const date deadline = sorted[static_cast<std::size_t>(row)].get_deadline();
    if (!confirm_action(this, "Sterge taskul", QString("Stergi taskul \"%1\"?").arg(QString::fromStdString(title)))) return;
    if (current_app.tasks_ref().remove_task(title, deadline)) {
      mgr.save_user_app(current_app, current_initial_balance);
      refresh_all();
    }
  });

  ll->addWidget(tasks_table, 1);
  ll->addLayout(button_row({reopen_task_btn, del_task_btn}));
  ll->addWidget(make_accent_header("Smart Focus Plan (Top Urgency Score)"));
  ll->addWidget(tasks_focus_box);

  QFrame* right_card = new QFrame{};
  right_card->setObjectName("Card");
  add_neumorphic_shadow(right_card, 24, 6, 180);
  right_card->setFixedWidth(355);
  QVBoxLayout* rl = new QVBoxLayout{right_card};
  rl->setContentsMargins(18, 16, 18, 16);

  rl->addWidget(make_accent_header("Adauga Task Nou"));
  task_title_edit = new QLineEdit{};
  task_title_edit->setPlaceholderText("Titlu task...");
  task_desc_edit = new QLineEdit{};
  task_desc_edit->setPlaceholderText("Descriere scurta...");
  task_level_spin = new QSpinBox{};
  task_level_spin->setRange(1, 3);
  task_level_spin->setValue(1);
  task_date_edit = new QLineEdit{"25.11.2026"};
  task_prog_spin = new QDoubleSpinBox{};
  task_prog_spin->setRange(0, 100);
  task_prog_spin->setValue(0);

  QPushButton* add_t_btn = new QPushButton{"＋  ADAUGA TASK"};
  connect(add_t_btn, &QPushButton::clicked, this, [this]() {
    date d{25, 11, 2026};
    parse_line(task_date_edit->text().toStdString(), d);
    if (!task_title_edit->text().isEmpty()) {
      current_app.add_task(task{
        task_title_edit->text().toStdString(),
                           task_desc_edit->text().isEmpty() ? "general" : task_desc_edit->text().toStdString(),
                           task_level_spin->value(),
                           d,
                           task_prog_spin->value()
      });
      mgr.save_user_app(current_app, current_initial_balance);
      task_title_edit->clear();
      task_desc_edit->clear();
      refresh_all();
    }
  });

  task_complete_combo = new QComboBox{};
  QPushButton* comp_btn = new QPushButton{"✓  MARCHEAZA CA FINALIZAT"};
  connect(comp_btn, &QPushButton::clicked, this, [this]() {
    std::string t = task_complete_combo->currentText().toStdString();
    if (!t.empty() && current_app.tasks_ref().complete(t)) {
      mgr.save_user_app(current_app, current_initial_balance);
      refresh_all();
    }
  });

  rank_level_spin = new QSpinBox{};
  rank_level_spin->setRange(1, 3);
  rank_level_spin->setValue(1);
  QPushButton* rank_btn = new QPushButton{"⚖   COMPARATIE BINARA (RANK)"};
  rank_btn->setObjectName("SecondaryBtn");
  connect(rank_btn, &QPushButton::clicked, this, [this]() {
    handle_binary_rank_gui(rank_level_spin->value());
  });

  rl->addWidget(new QLabel{"Titlu:"});
  rl->addWidget(task_title_edit);
  rl->addWidget(new QLabel{"Descriere:"});
  rl->addWidget(task_desc_edit);
  rl->addWidget(new QLabel{"Nivel Urgenta (1 = critic, 3 = scazut):"});
  rl->addWidget(task_level_spin);
  rl->addWidget(new QLabel{"Deadline (z.l.a):"});
  rl->addWidget(task_date_edit);
  rl->addWidget(new QLabel{"Progres Initial (%):"});
  rl->addWidget(task_prog_spin);
  rl->addWidget(add_t_btn);
  rl->addSpacing(14);
  rl->addWidget(make_accent_header("Finalizare & Sortare Binara"));
  rl->addWidget(task_complete_combo);
  rl->addWidget(comp_btn);
  rl->addSpacing(8);
  rl->addWidget(new QLabel{"Alege nivelul (1-3) pentru sortare binara:"});
  rl->addWidget(rank_level_spin);
  rl->addWidget(rank_btn);
  rl->addStretch();

  h->addWidget(left_card, 1);
  h->addWidget(right_card);
  return w;
}

QWidget* gui_window::build_calendar_tab() {
  QWidget* w = new QWidget{};
  QHBoxLayout* h = new QHBoxLayout{w};
  h->setContentsMargins(0, 0, 0, 0);
  h->setSpacing(14);

  QFrame* left_card = new QFrame{};
  left_card->setObjectName("Card");
  add_neumorphic_shadow(left_card, 24, 6, 180);
  QVBoxLayout* ll = new QVBoxLayout{left_card};
  ll->setContentsMargins(18, 16, 18, 16);

  ll->addWidget(make_accent_header("Evenimente Calendar & Status Sincronizare"));
  events_table = new QTableWidget{0, 5};
  events_table->setHorizontalHeaderLabels({"Eveniment", "Data", "Ora Start", "Durata (min)", "Status Google Sync"});
  events_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  events_table->setAlternatingRowColors(true);

  cal_report_box = new QTextEdit{};
  cal_report_box->setReadOnly(true);
  cal_report_box->setMaximumHeight(170);
  cal_report_box->setStyleSheet("font-family: 'JetBrains Mono', monospace;");

  setup_selectable(events_table);
  QPushButton* del_ev_btn = make_fix_button("🗑  STERGE EVENIMENTUL SELECTAT", true);
  connect(del_ev_btn, &QPushButton::clicked, this, [this]() {
    int row = selected_row(events_table);
    const auto& evs = current_app.calendar_ref().get_events();
    if (row < 0 || row >= static_cast<int>(evs.size())) {
      fix_hint(this, "Selecteaza intai un eveniment din tabel.");
      return;
    }
    QString name = QString::fromStdString(evs[static_cast<std::size_t>(row)].get_title());
    if (!confirm_action(this, "Sterge evenimentul", QString("Stergi evenimentul \"%1\"?").arg(name))) return;
    if (current_app.calendar_ref().remove_event(static_cast<std::size_t>(row))) {
      mgr.save_user_app(current_app, current_initial_balance);
      refresh_all();
    }
  });

  ll->addWidget(events_table, 1);
  ll->addLayout(button_row({del_ev_btn}));
  ll->addWidget(make_accent_header("Agenda Saptamanii & Jurnal Conflicte"));
  ll->addWidget(cal_report_box);

  QFrame* right_card = new QFrame{};
  right_card->setObjectName("Card");
  add_neumorphic_shadow(right_card, 24, 6, 180);
  right_card->setFixedWidth(345);
  QVBoxLayout* rl = new QVBoxLayout{right_card};
  rl->setContentsMargins(18, 16, 18, 16);

  rl->addWidget(make_accent_header("Programeaza Eveniment"));
  ev_title_edit = new QLineEdit{};
  ev_title_edit->setPlaceholderText("Ex: Antrenament / Sedinta...");
  ev_date_edit = new QLineEdit{"18.11.2026"};
  ev_hour_spin = new QSpinBox{};
  ev_hour_spin->setRange(0, 23);
  ev_hour_spin->setValue(14);
  ev_dur_spin = new QSpinBox{};
  ev_dur_spin->setRange(15, 480);
  ev_dur_spin->setValue(60);

  QPushButton* add_ev_btn = new QPushButton{"＋  ADAUGA EVENIMENT"};
  connect(add_ev_btn, &QPushButton::clicked, this, [this]() {
    date d{18, 11, 2026};
    parse_line(ev_date_edit->text().toStdString(), d);
    if (!ev_title_edit->text().isEmpty()) {
      current_app.add_event(calendar_event{
        ev_title_edit->text().toStdString(), d, ev_hour_spin->value(), ev_dur_spin->value(), false
      });
      mgr.save_user_app(current_app, current_initial_balance);
      ev_title_edit->clear();
      refresh_all();
    }
  });

  QPushButton* resolve_btn = new QPushButton{"⚙   AUTO-REZOLVA CONFLICTE"};
  resolve_btn->setObjectName("SecondaryBtn");
  connect(resolve_btn, &QPushButton::clicked, this, [this]() {
    int fixed = current_app.calendar_ref().auto_resolve_conflicts();
    mgr.save_user_app(current_app, current_initial_balance);
    refresh_all();
    QMessageBox::information(this, "Calendar", QString("Conflicte reprogramate automat: %1").arg(fixed));
  });

  QPushButton* sync_btn = new QPushButton{"⟳   SYNC DEADLINES & EXPORT .ICS"};
  connect(sync_btn, &QPushButton::clicked, this, [this]() {
    int pushed = current_app.sync_deadlines("data/gcal_sync.ics");
    mgr.save_user_app(current_app, current_initial_balance);
    refresh_all();
    QMessageBox::information(this, "Google Calendar Sync",
                             QString("Au fost sincronizate %1 evenimente in fisierul data/gcal_sync.ics!").arg(pushed));
  });

  rl->addWidget(new QLabel{"Titlu Eveniment:"});
  rl->addWidget(ev_title_edit);
  rl->addWidget(new QLabel{"Data (z.l.a):"});
  rl->addWidget(ev_date_edit);
  rl->addWidget(new QLabel{"Ora de Inceput (0-23):"});
  rl->addWidget(ev_hour_spin);
  rl->addWidget(new QLabel{"Durata (minute):"});
  rl->addWidget(ev_dur_spin);
  rl->addWidget(add_ev_btn);
  rl->addSpacing(20);
  rl->addWidget(make_accent_header("Actiuni Inteligente Calendar"));
  rl->addWidget(resolve_btn);
  rl->addWidget(sync_btn);
  rl->addStretch();

  h->addWidget(left_card, 1);
  h->addWidget(right_card);
  return w;
}

// ============================================================================
// Tab-ul Roadmap Vizual (Stil Obsidian Graph & Multi-Skill Tree)
// ============================================================================
QWidget* gui_window::build_roadmap_tab() {
  QWidget* w = new QWidget{};
  QHBoxLayout* h = new QHBoxLayout{w};
  h->setContentsMargins(0, 0, 0, 0);
  h->setSpacing(14);

  // Left Card: Interactive Graph Canvas + Selected Node Inspector
  QFrame* left_card = new QFrame{};
  left_card->setObjectName("Card");
  add_neumorphic_shadow(left_card, 24, 6, 180);
  QVBoxLayout* ll = new QVBoxLayout{left_card};
  ll->setContentsMargins(18, 16, 18, 16);
  ll->setSpacing(10);

  QHBoxLayout* top_bar = new QHBoxLayout{};
  top_bar->addWidget(make_accent_header("Harta Vizuala Skill-Tree & Conexiuni Noduri (Obsidian Graph)"));
  top_bar->addStretch();

  QPushButton* center_btn = new QPushButton{"⌖  Centreaza Graf"};
  center_btn->setObjectName("SecondaryBtn");
  center_btn->setFixedHeight(32);
  top_bar->addWidget(center_btn);
  ll->addLayout(top_bar);

  roadmap_graph_canvas = new RoadmapGraphWidget{};
  roadmap_graph_canvas->setStyleSheet("border: 1px solid #2e3130; border-radius: 12px;");
  connect(center_btn, &QPushButton::clicked, this, [this]() {
    roadmap_graph_canvas->reset_camera();
  });

  roadmap_graph_canvas->set_callbacks(
    [this](int clicked_id) {
      selected_rm_node_id = clicked_id;
      update_roadmap_inspector();
      // Selectam automat acest nod si in combo-ul de Parinte ca sa fie usor sa adaugi sub-task-uri
      for (const auto& n : current_app.roadmap_ref().get_nodes()) {
        if (n.get_id() == clicked_id) {
          int idx = rm_parent_combo->findText(QString::fromStdString(n.get_title()));
          if (idx >= 0) rm_parent_combo->setCurrentIndex(idx);
          break;
        }
      }
    },
    [this](const QString& node_title) {
      const std::string t = node_title.toStdString();
      int cnt = 1;
      for (const auto& n : current_app.roadmap_ref().get_nodes()) {
        if (n.get_title() == t && !n.is_done()) {
          cnt = current_app.roadmap_ref().subtree_count(n.get_id());
          break;
        }
      }
      if (cnt > 1 && !confirm_action(this, "Finalizeaza sub-arbore",
            QString("Nodul are sub-noduri. Finalizezi toate cele %1 noduri din sub-arbore?").arg(cnt))) {
        return;
      }
      if (current_app.roadmap_ref().complete_step(t)) {
        mgr.save_user_app(current_app, current_initial_balance);
        refresh_all();
      }
    }
  );

  ll->addWidget(roadmap_graph_canvas, 3);

  // Bottom Inspector Box for Selected Node & its Sub-tasks
  QFrame* insp_box = new QFrame{};
  insp_box->setStyleSheet("background-color: #181a19; border: 1px solid #2e3130; border-radius: 12px;");
  QVBoxLayout* il = new QVBoxLayout{insp_box};
  il->setContentsMargins(14, 10, 14, 10);
  il->setSpacing(6);

  rm_selected_title_lbl = new QLabel{"◈  Selecteaza un nod (cerc) din graf pentru a vedea sub-task-urile"};
  rm_selected_title_lbl->setStyleSheet("color: #00e65b; font-weight: 800; font-size: 13.5px;");

  rm_selected_meta_lbl = new QLabel{"Poti avea oricati arbori independenti (Skill Trees) pe acelasi canvas."};
  rm_selected_meta_lbl->setStyleSheet("color: #9ca3af; font-size: 12px;");

  rm_subtasks_list = new QListWidget{};
  rm_subtasks_list->setMaximumHeight(110);
  connect(rm_subtasks_list, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem* item) {
    int child_id = item->data(Qt::UserRole).toInt();
    if (child_id > 0) {
      selected_rm_node_id = child_id;
      roadmap_graph_canvas->set_selected_id(child_id);
      update_roadmap_inspector();
    }
  });

  il->addWidget(rm_selected_title_lbl);
  il->addWidget(rm_selected_meta_lbl);
  il->addWidget(rm_subtasks_list);
  ll->addWidget(insp_box, 1);

  // Right Card: Controls for Adding Sub-nodes, Creating New Independent Trees & Completing Steps
  QFrame* right_card = new QFrame{};
  right_card->setObjectName("Card");
  add_neumorphic_shadow(right_card, 24, 6, 180);
  right_card->setFixedWidth(360);
  QVBoxLayout* rl = new QVBoxLayout{right_card};
  rl->setContentsMargins(18, 16, 18, 16);
  rl->setSpacing(8);

  rl->addWidget(make_accent_header("Adauga Sub-Nod (sau Radacina)"));
  rm_parent_combo = new QComboBox{};
  rm_sub_edit = new QLineEdit{};
  rm_sub_edit->setPlaceholderText("Titlu nod / sub-task nou...");
  rm_weight_spin = new QDoubleSpinBox{};
  rm_weight_spin->setRange(0.1, 20.0);
  rm_weight_spin->setSingleStep(0.5);
  rm_weight_spin->setValue(1.5);

  QPushButton* split_btn = new QPushButton{"＋  ADAUGA NOD IN GRAF"};
  connect(split_btn, &QPushButton::clicked, this, [this]() {
    if (!rm_sub_edit->text().isEmpty()) {
      current_app.roadmap_ref().split_problem(
        rm_parent_combo->currentText().toStdString(),
                                              rm_sub_edit->text().toStdString(),
                                              rm_weight_spin->value()
      );
      mgr.save_user_app(current_app, current_initial_balance);
      rm_sub_edit->clear();
      refresh_all();
    }
  });

  rl->addWidget(new QLabel{"Parinte (alege ★ ca sa creezi o radacina / arbore nou):"});
  rl->addWidget(rm_parent_combo);
  rl->addWidget(new QLabel{"Titlu Nod Nou:"});
  rl->addWidget(rm_sub_edit);
  rl->addWidget(new QLabel{"Pondere Efort (Weight):"});
  rl->addWidget(rm_weight_spin);
  rl->addWidget(split_btn);

  rl->addSpacing(12);
  rl->addWidget(make_accent_header("Finalizeaza / Corecteaza Nod"));
  rm_complete_combo = new QComboBox{};
  QPushButton* comp_step_btn = new QPushButton{"✓  MARCHEAZA PASUL FINALIZAT"};
  connect(comp_step_btn, &QPushButton::clicked, this, [this]() {
    std::string st = rm_complete_combo->currentText().toStdString();
    if (!st.empty() && current_app.roadmap_ref().complete_step(st)) {
      mgr.save_user_app(current_app, current_initial_balance);
      refresh_all();
    }
  });

  QPushButton* reopen_node_btn = new QPushButton{"↩  REDESCHIDE NODUL SELECTAT"};
  reopen_node_btn->setObjectName("SecondaryBtn");
  connect(reopen_node_btn, &QPushButton::clicked, this, [this]() {
    if (selected_rm_node_id < 0) {
      fix_hint(this, "Selecteaza intai un nod (click pe un cerc din graf).");
      return;
    }
    int cnt = current_app.roadmap_ref().subtree_count(selected_rm_node_id);
    if (cnt > 1 && !confirm_action(this, "Redeschide nod",
          QString("Nodul selectat are sub-noduri. Se redeschid toate cele %1 noduri din sub-arbore. Continui?").arg(cnt))) {
      return;
    }
    if (current_app.roadmap_ref().reopen_node(selected_rm_node_id)) {
      mgr.save_user_app(current_app, current_initial_balance);
      refresh_all();
    } else {
      fix_hint(this, "Nodul selectat nu este marcat ca finalizat.");
    }
  });

  QPushButton* del_node_btn = new QPushButton{"🗑  STERGE NODUL SELECTAT"};
  del_node_btn->setObjectName("SecondaryBtn");
  del_node_btn->setStyleSheet("color: #ff7b7b;");
  connect(del_node_btn, &QPushButton::clicked, this, [this]() {
    if (selected_rm_node_id < 0) {
      fix_hint(this, "Selecteaza intai un nod (click pe un cerc din graf).");
      return;
    }
    QString name;
    for (const auto& n : current_app.roadmap_ref().get_nodes()) {
      if (n.get_id() == selected_rm_node_id) {
        name = QString::fromStdString(n.get_title());
        break;
      }
    }
    int cnt = current_app.roadmap_ref().subtree_count(selected_rm_node_id);
    QString msg = QString("Stergi nodul \"%1\"").arg(name);
    if (cnt > 1) msg += QString(" impreuna cu cele %1 sub-noduri ale lui").arg(cnt - 1);
    msg += "?";
    if (!confirm_action(this, "Sterge nod", msg)) return;
    if (current_app.roadmap_ref().remove_node(selected_rm_node_id) > 0) {
      selected_rm_node_id = -1;
      mgr.save_user_app(current_app, current_initial_balance);
      refresh_all();
    }
  });

  rl->addWidget(rm_complete_combo);
  rl->addWidget(comp_step_btn);
  rl->addSpacing(6);
  rl->addWidget(new QLabel{"Corectii (actioneaza pe nodul selectat din graf):"});
  rl->addWidget(reopen_node_btn);
  rl->addWidget(del_node_btn);
  rl->addStretch();

  h->addWidget(left_card, 1);
  h->addWidget(right_card);
  return w;
}

QWidget* gui_window::build_habits_fitness_tab() {
  QWidget* w = new QWidget{};
  QHBoxLayout* h = new QHBoxLayout{w};
  h->setContentsMargins(0, 0, 0, 0);
  h->setSpacing(14);

  QFrame* left_card = new QFrame{};
  left_card->setObjectName("Card");
  add_neumorphic_shadow(left_card, 24, 6, 180);
  QVBoxLayout* ll = new QVBoxLayout{left_card};
  ll->setContentsMargins(18, 16, 18, 16);

  ll->addWidget(make_accent_header("Obiceiuri Zilnice & Streak-uri (Habit Tracker)"));
  habits_table = new QTableWidget{0, 6};
  habits_table->setHorizontalHeaderLabels({"Obicei", "Categorie", "Tinta/Sapt", "Streak Curent", "Best Streak", "Consistenta"});
  habits_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

  setup_selectable(habits_table);
  QPushButton* undo_hab_btn = make_fix_button("↩  ANULEAZA CHECK-IN DE AZI", false);
  QPushButton* del_hab_btn = make_fix_button("🗑  STERGE OBICEIUL SELECTAT", true);
  connect(undo_hab_btn, &QPushButton::clicked, this, [this]() {
    int row = selected_row(habits_table);
    const auto& habs = current_app.habits_ref().get_habits();
    if (row < 0 || row >= static_cast<int>(habs.size())) {
      fix_hint(this, "Selecteaza intai un obicei din tabel.");
      return;
    }
    const std::string title = habs[static_cast<std::size_t>(row)].get_title();
    if (current_app.undo_check_in_habit(title)) {
      mgr.save_user_app(current_app, current_initial_balance);
      refresh_all();
    } else {
      fix_hint(this, "Obiceiul selectat nu are niciun check-in facut la data curenta, deci nu e nimic de anulat.");
    }
  });
  connect(del_hab_btn, &QPushButton::clicked, this, [this]() {
    int row = selected_row(habits_table);
    const auto& habs = current_app.habits_ref().get_habits();
    if (row < 0 || row >= static_cast<int>(habs.size())) {
      fix_hint(this, "Selecteaza intai un obicei din tabel.");
      return;
    }
    QString name = QString::fromStdString(habs[static_cast<std::size_t>(row)].get_title());
    if (!confirm_action(this, "Sterge obiceiul", QString("Stergi obiceiul \"%1\" (cu tot cu streak)?").arg(name))) return;
    if (current_app.habits_ref().remove_habit(static_cast<std::size_t>(row))) {
      mgr.save_user_app(current_app, current_initial_balance);
      refresh_all();
    }
  });

  ll->addWidget(habits_table, 1);
  ll->addLayout(button_row({undo_hab_btn, del_hab_btn}));
  ll->addWidget(make_accent_header("Sesiuni Antrenament & Training Load (Fitness Tracker)"));

  workouts_table = new QTableWidget{0, 6};
  workouts_table->setHorizontalHeaderLabels({"Antrenament", "Sport", "Data", "Durata (min)", "Calorii", "Training Load"});
  workouts_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

  setup_selectable(workouts_table);
  QPushButton* del_ws_btn = make_fix_button("🗑  STERGE ANTRENAMENTUL SELECTAT", true);
  connect(del_ws_btn, &QPushButton::clicked, this, [this]() {
    int row = selected_row(workouts_table);
    const auto& wss = current_app.fitness_ref().get_sessions();
    if (row < 0 || row >= static_cast<int>(wss.size())) {
      fix_hint(this, "Selecteaza intai un antrenament din tabel.");
      return;
    }
    QString name = QString::fromStdString(wss[static_cast<std::size_t>(row)].get_title());
    if (!confirm_action(this, "Sterge antrenamentul", QString("Stergi antrenamentul \"%1\"?").arg(name))) return;
    if (current_app.fitness_ref().remove_session(static_cast<std::size_t>(row))) {
      mgr.save_user_app(current_app, current_initial_balance);
      refresh_all();
    }
  });

  ll->addWidget(workouts_table, 1);
  ll->addLayout(button_row({del_ws_btn}));

  QFrame* right_card = new QFrame{};
  right_card->setObjectName("Card");
  add_neumorphic_shadow(right_card, 24, 6, 180);
  right_card->setFixedWidth(355);
  QVBoxLayout* rl = new QVBoxLayout{right_card};
  rl->setContentsMargins(18, 16, 18, 16);

  rl->addWidget(make_accent_header("Check-In sau Adauga Obicei"));
  hab_checkin_combo = new QComboBox{};
  QPushButton* checkin_btn = new QPushButton{"✓  CHECK-IN AZI (+1 STREAK)"};
  connect(checkin_btn, &QPushButton::clicked, this, [this]() {
    std::string ht = hab_checkin_combo->currentText().toStdString();
    if (!ht.empty() && current_app.check_in_habit(ht)) {
      mgr.save_user_app(current_app, current_initial_balance);
      refresh_all();
    }
  });

  hab_title_edit = new QLineEdit{};
  hab_title_edit->setPlaceholderText("Titlu obicei nou...");
  hab_cat_edit = new QLineEdit{"health"};
  hab_days_spin = new QSpinBox{};
  hab_days_spin->setRange(1, 7);
  hab_days_spin->setValue(5);

  QPushButton* add_hab_btn = new QPushButton{"＋  ADAUGA OBICEI"};
  add_hab_btn->setObjectName("SecondaryBtn");
  connect(add_hab_btn, &QPushButton::clicked, this, [this]() {
    if (!hab_title_edit->text().isEmpty()) {
      current_app.add_habit(habit{
        hab_title_edit->text().toStdString(),
                            hab_cat_edit->text().toStdString(),
                            hab_days_spin->value(),
                            1, 1, current_app.get_date()
      });
      mgr.save_user_app(current_app, current_initial_balance);
      hab_title_edit->clear();
      refresh_all();
    }
  });

  ws_title_edit = new QLineEdit{};
  ws_title_edit->setPlaceholderText("Ex: Bouldering / Ciclism...");
  ws_sport_edit = new QLineEdit{"Bouldering"};
  ws_dur_spin = new QSpinBox{};
  ws_dur_spin->setRange(5, 600);
  ws_dur_spin->setValue(90);
  ws_cal_spin = new QSpinBox{};
  ws_cal_spin->setRange(10, 5000);
  ws_cal_spin->setValue(550);
  ws_int_spin = new QSpinBox{};
  ws_int_spin->setRange(1, 10);
  ws_int_spin->setValue(8);

  QPushButton* add_ws_btn = new QPushButton{"＋  ADAUGA ANTRENAMENT"};
  connect(add_ws_btn, &QPushButton::clicked, this, [this]() {
    if (!ws_title_edit->text().isEmpty()) {
      current_app.add_workout(workout_session{
        ws_title_edit->text().toStdString(),
                              ws_sport_edit->text().toStdString(),
                              ws_dur_spin->value(),
                              ws_cal_spin->value(),
                              ws_int_spin->value(),
                              current_app.get_date()
      });
      mgr.save_user_app(current_app, current_initial_balance);
      ws_title_edit->clear();
      refresh_all();
    }
  });

  rl->addWidget(hab_checkin_combo);
  rl->addWidget(checkin_btn);
  rl->addWidget(hab_title_edit);
  rl->addWidget(hab_cat_edit);
  rl->addWidget(hab_days_spin);
  rl->addWidget(add_hab_btn);
  rl->addSpacing(12);
  rl->addWidget(make_accent_header("Inregistreaza Antrenament"));
  rl->addWidget(ws_title_edit);
  rl->addWidget(ws_sport_edit);
  rl->addWidget(new QLabel{"Durata (min) / Calorii / Intensitate (1-10):"});
  QHBoxLayout* metrics_h = new QHBoxLayout{};
  metrics_h->addWidget(ws_dur_spin);
  metrics_h->addWidget(ws_cal_spin);
  metrics_h->addWidget(ws_int_spin);
  rl->addLayout(metrics_h);
  rl->addWidget(add_ws_btn);
  rl->addStretch();

  h->addWidget(left_card, 1);
  h->addWidget(right_card);
  return w;
}

void gui_window::switch_nav_tab(int idx) {
  module_stack->setCurrentIndex(idx);
  for (std::size_t i = 0; i < nav_buttons.size(); ++i) {
    nav_buttons[i]->setObjectName(static_cast<int>(i) == idx ? "NavBtnActive" : "NavBtn");
    nav_buttons[i]->style()->unpolish(nav_buttons[i]);
    nav_buttons[i]->style()->polish(nav_buttons[i]);
  }
  refresh_all();
}

void gui_window::handle_login() {
  std::string u = login_user_edit->text().trimmed().toStdString();
  std::string p = login_pass_edit->text().toStdString();
  account_record rec;
  if (mgr.authenticate(u, p, rec)) {
    current_initial_balance = rec.initial_balance;
    current_app = mgr.load_user_app(rec);
    logged_in = true;
    auth_status_lbl->clear();
    refresh_all();
    root_stack->setCurrentIndex(1);
  } else {
    auth_status_lbl->setText("✕  Utilizator sau parola incorecta!");
  }
}

void gui_window::handle_register() {
  std::string u = reg_user_edit->text().trimmed().toStdString();
  std::string p = reg_pass_edit->text().toStdString();
  date d{17, 11, 2026};
  parse_line(reg_date_edit->text().toStdString(), d);
  double bal = reg_balance_spin->value();
  bool seed = reg_seed_check->isChecked();

  if (u.empty() || p.empty()) {
    auth_status_lbl->setText("✕  Completeaza numele de utilizator si parola!");
    return;
  }
  if (!mgr.register_account(u, p, d, bal, seed)) {
    auth_status_lbl->setText("✕  Acest nume de utilizator exista deja!");
    return;
  }

  account_record rec{u, p, d, bal};
  current_initial_balance = bal;
  current_app = mgr.load_user_app(rec);
  logged_in = true;
  auth_status_lbl->clear();
  refresh_all();
  root_stack->setCurrentIndex(1);
}

void gui_window::handle_logout() {
  if (logged_in) {
    mgr.save_user_app(current_app, current_initial_balance);
    logged_in = false;
  }
  root_stack->setCurrentIndex(0);
}

void gui_window::handle_next_day() {
  current_app.next_day();
  mgr.save_user_app(current_app, current_initial_balance);
  refresh_all();
}

void gui_window::handle_binary_rank_gui(int level) {
  std::vector<task> all = current_app.tasks_ref().get_tasks();
  std::vector<task> ranked, others;
  for (const auto& t : all) {
    if (!t.has_level(level)) {
      others.push_back(t);
      continue;
    }
    std::size_t lo = 0, hi = ranked.size();
    while (lo < hi) {
      std::size_t mid = (lo + hi) / 2;
      int diff = t.deadline_diff(ranked[mid]);
      QString q = QString(
        "Comparatie Binara (Nivel %1):\n\n"
        "Sarcina A: %2 (Deadline: %3)\n"
        "Sarcina B: %4 (Deadline: %5)\n"
        "Diferenta deadline: %6 zile\n\n"
        "Este \"%2\" mai importanta decat \"%4\"?"
      ).arg(level)
      .arg(QString::fromStdString(t.get_title()))
      .arg(date_qstr(t.get_deadline()))
      .arg(QString::fromStdString(ranked[mid].get_title()))
      .arg(date_qstr(ranked[mid].get_deadline()))
      .arg(diff);

      auto ans = QMessageBox::question(this, "Prioritizare Binara", q, QMessageBox::Yes | QMessageBox::No);
      if (ans == QMessageBox::Yes) hi = mid;
      else lo = mid + 1;
    }
    ranked.insert(ranked.begin() + static_cast<std::ptrdiff_t>(lo), t);
  }
  others.insert(others.end(), ranked.begin(), ranked.end());
  current_app.tasks_ref().set_tasks(others);
  mgr.save_user_app(current_app, current_initial_balance);
  refresh_all();
}

void gui_window::update_roadmap_inspector() {
  const auto& nodes = current_app.roadmap_ref().get_nodes();
  rm_subtasks_list->clear();

  if (nodes.empty()) {
    rm_selected_title_lbl->setText("◈  Niciun nod in Roadmap");
    rm_selected_meta_lbl->setText("Foloseste panoul din dreapta pentru a crea un arbore nou.");
    return;
  }

  // Daca nu e selectat nimic, selectam primul nod radacina
  const roadmap_node* target = nullptr;
  for (const auto& n : nodes) {
    if (n.get_id() == selected_rm_node_id) {
      target = &n;
      break;
    }
  }
  if (!target) {
    target = &nodes.front();
    selected_rm_node_id = target->get_id();
  }

  double prog = current_app.roadmap_ref().node_progress(target->get_id());
  double rem = current_app.roadmap_ref().node_remaining_effort(target->get_id());

  QString type_badge = (target->get_parent() == 0) ? "★ RADACINA ARBORE (SKILL TREE)" : QString("Sub-nod (Parinte #%1)").arg(target->get_parent());
  rm_selected_title_lbl->setText(QString("◈  Nod Selectat: %1   [%2]").arg(
    QString::fromStdString(target->get_title()),
                                                                           target->is_done() ? "FINALIZAT ✓" : QString("%1%").arg(prog, 0, 'f', 0)
  ));

  rm_selected_meta_lbl->setText(QString("%1   |   Pondere proprie: %2   |   Efort ramas in sub-arbore: %3   |   Urmatorul pas recomandat: %4")
  .arg(type_badge)
  .arg(target->get_weight(), 0, 'f', 1)
  .arg(rem, 0, 'f', 1)
  .arg(QString::fromStdString(current_app.roadmap_ref().next_step()))
  );

  int child_count = 0;
  for (const auto& n : nodes) {
    if (n.get_parent() == target->get_id()) {
      ++child_count;
      double c_prog = current_app.roadmap_ref().node_progress(n.get_id());
      QString line = QString("%1  #%2  %3   —   Progres: %4%   (pondere %5)")
      .arg(n.is_done() ? "[✓]" : "[  ]")
      .arg(n.get_id())
      .arg(QString::fromStdString(n.get_title()))
      .arg(c_prog, 0, 'f', 0)
      .arg(n.get_weight(), 0, 'f', 1);
      auto* item = new QListWidgetItem{line};
      item->setData(Qt::UserRole, n.get_id());
      item->setForeground(n.is_done() ? QColor{"#00e65b"} : QColor{"#f4f4f6"});
      rm_subtasks_list->addItem(item);
    }
  }

  if (child_count == 0) {
    auto* empty_item = new QListWidgetItem{"Acest nod este o frunza (nu are sub-task-uri). Poti adauga sub-task-uri in dreapta sau dublu-click pe cerc pentru a-l finaliza."};
    empty_item->setForeground(QColor{"#8e9290"});
    rm_subtasks_list->addItem(empty_item);
  }
}

void gui_window::refresh_all() {
  if (!logged_in) return;

  std::ostringstream d_ss;
  d_ss << current_app.get_date() << " (" << current_app.get_date().weekday_name()
  << ", Q" << current_app.get_date().quarter() << ")";

  header_user_lbl->setText(QString("CONT: %1").arg(QString::fromStdString(current_app.get_user())));
  header_date_lbl->setText(QString("DATA: %1").arg(QString::fromStdString(d_ss.str())));
  int score = static_cast<int>(current_app.life_score());
  header_score_bar->setValue(score);

  double left_ron = current_app.budget_ref().get_left();
  double total_ron = std::max(1.0, current_app.budget_ref().get_total());
  int b_pct = std::clamp(static_cast<int>((left_ron / total_ron) * 100.0), 0, 100);
  int t_pct = std::clamp(static_cast<int>(current_app.tasks_ref().completion()), 0, 100);
  int r_pct = std::clamp(static_cast<int>(current_app.roadmap_ref().overall_progress()), 0, 100);
  int h_pct = std::clamp(static_cast<int>(current_app.habits_ref().average_consistency()), 0, 100);
  int f_pct = std::clamp(static_cast<int>(current_app.fitness_ref().goal_completion(current_app.get_date())), 0, 100);

  card_budget_val->setText(QString("%1 RON").arg(left_ron, 0, 'f', 0));
  card_tasks_val->setText(QString("%1%").arg(t_pct));
  card_roadmap_val->setText(QString("%1%").arg(r_pct));
  card_habits_val->setText(QString("%1%").arg(h_pct));
  card_fitness_val->setText(QString("%1%").arg(f_pct));

  if (card_vbars.size() == 5) {
    card_vbars[0]->setValue(b_pct);
    card_vbars[1]->setValue(t_pct);
    card_vbars[2]->setValue(r_pct);
    card_vbars[3]->setValue(h_pct);
    card_vbars[4]->setValue(f_pct);
  }

  // Capture lifetracker::summary() output
  {
    std::ostringstream cap;
    std::streambuf* old = std::cout.rdbuf(cap.rdbuf());
    current_app.summary();
    std::cout.rdbuf(old);
    dashboard_console->setPlainText(QString::fromStdString(cap.str()));
  }

  // Budget tables
  const auto& txs = current_app.budget_ref().get_transactions();
  tx_table->setRowCount(static_cast<int>(txs.size()));
  for (std::size_t i = 0; i < txs.size(); ++i) {
    int r = static_cast<int>(i);
    std::ostringstream dstr;
    dstr << txs[i].get_date();
    tx_table->setItem(r, 0, new QTableWidgetItem{QString::fromStdString(txs[i].get_title())});
    tx_table->setItem(r, 1, new QTableWidgetItem{QString::fromStdString(txs[i].category_name())});
    tx_table->setItem(r, 2, new QTableWidgetItem{QString::fromStdString(dstr.str())});
    tx_table->setItem(r, 3, new QTableWidgetItem{QString::number(txs[i].signed_amount(), 'f', 2)});
    tx_table->setItem(r, 4, new QTableWidgetItem{txs[i].is_membership() ? "ON" : "OFF"});
    tx_table->setItem(r, 5, new QTableWidgetItem{txs[i].is_win() ? "ON" : "OFF"});
  }

  const auto& goals = current_app.budget_ref().get_goals();
  goals_table->setRowCount(static_cast<int>(goals.size()));
  for (std::size_t i = 0; i < goals.size(); ++i) {
    int r = static_cast<int>(i);
    std::ostringstream dstr;
    dstr << goals[i].get_deadline();
    goals_table->setItem(r, 0, new QTableWidgetItem{QString::fromStdString(goals[i].get_title())});
    goals_table->setItem(r, 1, new QTableWidgetItem{QString::number(goals[i].get_money(), 'f', 0) + " RON"});
    goals_table->setItem(r, 2, new QTableWidgetItem{QString::fromStdString(dstr.str())});
    goals_table->setItem(r, 3, new QTableWidgetItem{QString::number(goals[i].days_left(current_app.get_date())) + " zile"});
  }

  {
    std::ostringstream cap;
    std::streambuf* old = std::cout.rdbuf(cap.rdbuf());
    current_app.budget_ref().period_report(current_app.get_date());
    current_app.budget_ref().spending_split(current_app.get_date());
    current_app.budget_ref().subscriptions_status(current_app.get_date());
    current_app.budget_ref().forecast_balance(3, current_app.get_date());
    std::cout.rdbuf(old);
    budget_report_box->setPlainText(QString::fromStdString(cap.str()));
  }

  // Tasks table
  auto sorted_tasks = current_app.tasks_ref().prioritize();
  tasks_table->setRowCount(static_cast<int>(sorted_tasks.size()));
  task_complete_combo->clear();
  for (std::size_t i = 0; i < sorted_tasks.size(); ++i) {
    int r = static_cast<int>(i);
    std::ostringstream dstr;
    dstr << sorted_tasks[i].get_deadline();
    tasks_table->setItem(r, 0, new QTableWidgetItem{QString::fromStdString(sorted_tasks[i].get_title())});
    tasks_table->setItem(r, 1, new QTableWidgetItem{QString::fromStdString(sorted_tasks[i].get_description())});
    tasks_table->setItem(r, 2, new QTableWidgetItem{QString("Nivel %1").arg(sorted_tasks[i].get_urgency())});
    tasks_table->setItem(r, 3, new QTableWidgetItem{QString::fromStdString(dstr.str())});
    tasks_table->setItem(r, 4, new QTableWidgetItem{QString("%1%").arg(sorted_tasks[i].get_progress(), 0, 'f', 0)});
    QString st = sorted_tasks[i].is_done() ? "ON (Done)" : (sorted_tasks[i].is_missed() ? "RESTANT" : "IN LUCRU");
    tasks_table->setItem(r, 5, new QTableWidgetItem{st});
    if (!sorted_tasks[i].is_done()) {
      task_complete_combo->addItem(QString::fromStdString(sorted_tasks[i].get_title()));
    }
  }

  {
    std::ostringstream cap;
    std::streambuf* old = std::cout.rdbuf(cap.rdbuf());
    current_app.tasks_ref().focus_plan(current_app.get_date(), 5);
    std::cout.rdbuf(old);
    tasks_focus_box->setPlainText(QString::fromStdString(cap.str()));
  }

  // Calendar table
  const auto& evs = current_app.calendar_ref().get_events();
  events_table->setRowCount(static_cast<int>(evs.size()));
  for (std::size_t i = 0; i < evs.size(); ++i) {
    int r = static_cast<int>(i);
    std::ostringstream dstr;
    dstr << evs[i].get_date();
    events_table->setItem(r, 0, new QTableWidgetItem{QString::fromStdString(evs[i].get_title())});
    events_table->setItem(r, 1, new QTableWidgetItem{QString::fromStdString(dstr.str())});
    events_table->setItem(r, 2, new QTableWidgetItem{QString("%1:00").arg(evs[i].get_start_hour())});
    events_table->setItem(r, 3, new QTableWidgetItem{QString("%1 min").arg(evs[i].get_duration())});
    events_table->setItem(r, 4, new QTableWidgetItem{evs[i].is_synced() ? "ON (Google Synced)" : "OFF (Local)"});
  }

  {
    std::ostringstream cap;
    std::streambuf* old = std::cout.rdbuf(cap.rdbuf());
    current_app.calendar_ref().week_agenda(current_app.get_date());
    int c = current_app.calendar_ref().detect_conflicts();
    std::cout << "Total conflicte detectate: " << c << "\n";
    std::cout.rdbuf(old);
    cal_report_box->setPlainText(QString::fromStdString(cap.str()));
  }

  // Roadmap Interactive Graph & Inspector
  rm_parent_combo->clear();
  rm_complete_combo->clear();
  rm_parent_combo->addItem("★ [Arbore Nou / Radacina Independenta]");
  for (const auto& n : current_app.roadmap_ref().get_nodes()) {
    rm_parent_combo->addItem(QString::fromStdString(n.get_title()));
    if (!n.is_done()) {
      rm_complete_combo->addItem(QString::fromStdString(n.get_title()));
    }
  }

  roadmap_graph_canvas->rebuild_from_roadmap(current_app.roadmap_ref(), selected_rm_node_id);
  update_roadmap_inspector();

  // Habits & Fitness
  const auto& habs = current_app.habits_ref().get_habits();
  habits_table->setRowCount(static_cast<int>(habs.size()));
  hab_checkin_combo->clear();
  for (std::size_t i = 0; i < habs.size(); ++i) {
    int r = static_cast<int>(i);
    habits_table->setItem(r, 0, new QTableWidgetItem{QString::fromStdString(habs[i].get_title())});
    habits_table->setItem(r, 1, new QTableWidgetItem{QString::fromStdString(habs[i].get_category())});
    habits_table->setItem(r, 2, new QTableWidgetItem{QString("%1 zile/sapt").arg(habs[i].get_target_days())});
    habits_table->setItem(r, 3, new QTableWidgetItem{QString("%1 zile").arg(habs[i].get_streak())});
    habits_table->setItem(r, 4, new QTableWidgetItem{QString::number(habs[i].get_best_streak())});
    habits_table->setItem(r, 5, new QTableWidgetItem{QString("%1%").arg(habs[i].consistency_score(), 0, 'f', 0)});
    hab_checkin_combo->addItem(QString::fromStdString(habs[i].get_title()));
  }

  const auto& wss = current_app.fitness_ref().get_sessions();
  workouts_table->setRowCount(static_cast<int>(wss.size()));
  for (std::size_t i = 0; i < wss.size(); ++i) {
    int r = static_cast<int>(i);
    std::ostringstream dstr;
    dstr << wss[i].get_date();
    workouts_table->setItem(r, 0, new QTableWidgetItem{QString::fromStdString(wss[i].get_title())});
    workouts_table->setItem(r, 1, new QTableWidgetItem{QString::fromStdString(wss[i].get_sport())});
    workouts_table->setItem(r, 2, new QTableWidgetItem{QString::fromStdString(dstr.str())});
    workouts_table->setItem(r, 3, new QTableWidgetItem{QString("%1 min").arg(wss[i].get_duration())});
    workouts_table->setItem(r, 4, new QTableWidgetItem{QString("%1 kcal").arg(wss[i].get_calories())});
    workouts_table->setItem(r, 5, new QTableWidgetItem{QString::number(wss[i].training_load(), 'f', 1)});
  }
}

#endif
