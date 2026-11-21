# Mover, alinhar, agrupar e sobrepor objetos
<!-- keywords: painel de seleção, ocultar objeto, mostrar objeto, mudar o nome do objeto, nome do objeto, selecionar, mover, redimensionar, rodar, inverter, alinhar, distribuir, agrupar, desagrupar, camada, trazer para a frente, enviar para trás, deslocar, bloquear, duplicar -->

Cada imagem, caixa de texto, forma e linha numa página é um objeto. Este tópico mostra como selecionar, mover, redimensionar e sobrepor objetos.

## Selecionar objetos

- Clique num objeto. Para selecionar uma caixa de texto, clique dentro dela e, de seguida, prima **Esc**.
- Mantenha **Shift** premido e clique para adicionar um objeto à seleção, ou para o retirar.
- Arraste um retângulo à volta dos objetos numa parte vazia da página. Ficam selecionados os objetos que caibam completamente dentro do retângulo.
- Prima **Ctrl+A** para selecionar todos os objetos da página. Prima **Tab** para passar para o objeto seguinte.
- **Base > Edição > Selecionar** permite selecionar de uma só vez todas as caixas de texto, imagens, formas, tabelas ou Texto Artístico.

## Mover e redimensionar

- Arraste um objeto para o mover. Mantenha **Shift** premido para o mover apenas em linha reta na horizontal ou na vertical. Mantenha **Alt** premido para desativar o ajuste. Mantenha **Ctrl** premido enquanto arrasta para criar uma cópia.
- Prima as teclas de seta para deslocar o objeto 1/8 de polegada, ou seja, cerca de 0,32 cm. **Alt** com uma tecla de seta move o objeto um píxel do ecrã. Pode alterar a distância de deslocamento em **Ficheiro > Opções > Avançadas**.
- Arraste uma alça quadrada para redimensionar. Uma alça de canto altera a largura e a altura em simultâneo. Uma alça lateral altera apenas uma dimensão.
- Caixas de texto e formas: mantenha **Shift** premido enquanto arrasta um canto para manter as proporções.
- Imagens, Texto Artístico, ícones e grupos já mantêm as proporções ao arrastar um canto. Mantenha **Shift** premido para os esticar livremente.
- Mantenha **Ctrl** premido para redimensionar a partir do centro.
- Para valores exatos, escreva nas caixas **Altura** e **Largura** do grupo **Tamanho** (em separadores como **Formato da Forma**), ou clique com o botão direito no objeto, escolha **Formatar Objeto…** e use os separadores **Tamanho** e **Esquema**. **Ver > Ferramentas > Medição** abre uma pequena janela que mostra e define a posição, o tamanho e a rotação.

## Rodar e inverter

Arraste a alça verde redonda acima de um objeto selecionado. Mantenha **Shift** premido para rodar em passos de 15 graus. Ou clique em **Base > Organizar > Rodar** e escolha **Rodar 90° à Direita**, **Rodar 90° para a Esquerda**, **Inverter Verticalmente**, **Inverter Horizontalmente** ou **Mais Opções de Rotação…** para escrever um ângulo.

## Alinhar e distribuir

1. Selecione os objetos.
2. Clique em **Base > Organizar > Alinhar**.
3. Escolha Alinhar à Esquerda, Alinhar ao Centro, Alinhar à Direita, Alinhar em Cima, Alinhar ao Meio ou Alinhar em Baixo.

Com vários objetos selecionados, alinham-se entre si. Com um só objeto selecionado, alinha-se com as margens da página. Marque **Relativamente às Guias de Margem** no mesmo menu para usar, em vez disso, as guias de margem.

**Distribuir Horizontalmente** e **Distribuir Verticalmente** espaçam igualmente três ou mais objetos entre os dois mais exteriores.

## Dar o mesmo tamanho aos objetos

1. Selecione dois ou mais objetos. Selecione primeiro aquele cujo tamanho os outros devem adotar.
2. Clique em **Base > Organizar > Alinhar**.
3. Escolha **Igualar Largura**, **Igualar Altura** ou **Igualar Largura e Altura**.

Cada um dos outros objetos adota a largura e a altura desse (um objeto rodado adota a sua própria largura e altura, e não as do respetivo contorno rodado) e mantém o canto superior esquerdo onde estava. Um objeto bloqueado mantém o seu próprio tamanho. Cada escolha corresponde a um passo de Anular.

## Raio do canto e ponto de redimensionamento

**Formatar Objeto… > Tamanho** tem **Redimensionar a partir de**. Indica o ponto do objeto que se mantém no mesmo sítio quando altera a altura ou a largura: um canto, o meio de um lado ou o centro. A escolha habitual é **Superior esquerdo**. Num objeto rodado, o ponto mantém-se onde o vê na página. Se também escrever uma nova posição, prevalece a posição que escreveu. Para um retângulo arredondado (e para as formas com um ou dois cantos arredondados), **Raio do canto** define o grau de arredondamento dos cantos, na unidade que usar. O valor máximo é metade do lado mais curto, onde os cantos se encontram.

## Duplicar várias vezes

Ctrl+D cria uma cópia, deslocada 12 pontos para baixo e para a direita. Para criar mais, selecione os objetos e clique em **Base > Organizar > Duplicar Várias Vezes…** (ou clique com o botão direito no objeto e escolha-o). Na janela, defina:

- **Número de cópias**, de 1 a 200. Não são criados mais de 2000 novos objetos de cada vez, e as cópias param antes da primeira que ficaria com mais de 20 vezes o lado maior da página, com menos de um ponto ou deslocada além desse limite.
- **Deslocamento horizontal** e **Deslocamento vertical**: a distância a que cada cópia é deslocada em relação à anterior. Um número negativo desloca para a esquerda ou para cima.
- **Alteração de tamanho por cada cópia**, em percentagem. Por exemplo, 10 % faz com que cada cópia fique 10 % maior do que a anterior, e -10 % faz com que fique menor. O canto superior esquerdo de cada cópia fica onde o respetivo deslocamento a coloca.
- **Passar a cor de preenchimento para**: escolha uma cor, e as cópias mudam gradualmente do preenchimento sólido do objeto para essa cor, de modo que a última cópia tenha exatamente essa cor. Um objeto sem preenchimento sólido mantém o seu preenchimento.

Todas as cópias correspondem a um passo de Anular, e as últimas cópias ficam selecionadas.

## Enviar objetos para a página modelo

1. Selecione os objetos de uma página.
2. Clique em **Base > Organizar > Enviar para Página Modelo**.

Os objetos passam para a página modelo usada pela página, à frente do que lá já existe, e têm o mesmo aspeto: mantêm-se no mesmo sítio da página. Depois disso, aparecem em todas as páginas que usam essa página modelo, e altera-os na página modelo. Para os devolver, abra a página modelo (**Ver > Vistas > Página Modelo**), selecione-os e clique em **Enviar para a Página** (o mesmo botão); ficam na página onde se encontrava. Os objetos saem da página, pelo que aparecem em todas as páginas que usam essa página modelo, e não apenas nesta. Cada operação corresponde a um passo de Anular. Uma página sem página modelo, um objeto bloqueado, um objeto dentro de um grupo (selecione o grupo inteiro), uma caixa de texto ligada a outra e um objeto que acompanha a sua página (um gráfico ou um objeto incorporado) permanecem onde estão, e a barra de estado indica o motivo.

## Agrupar objetos

Um grupo move-se, redimensiona-se e roda como um único objeto.

1. Selecione dois ou mais objetos.
2. Clique em **Base > Organizar > Agrupar** (Ctrl+Shift+G).

Para desfazer um grupo, selecione-o e clique em **Desagrupar**. A seta ao lado de **Desagrupar** oferece **Reagrupar**, que volta a juntar o último grupo. Para selecionar um objeto dentro de um grupo, faça duplo clique nele. Prima **Esc** para voltar a selecionar o grupo inteiro.

## Alterar a ordem de sobreposição

Os objetos sobrepõem-se como folhas de papel. Use **Trazer para a Frente** e **Enviar para Trás** para mover um objeto um nível de cada vez. A seta ao lado de cada botão oferece **Trazer para Primeiro Plano** (Alt+F6) e **Enviar para Segundo Plano** (Alt+Shift+F6). Clique com o botão direito e escolha **Ordem** para as mesmas opções.

## Listar os objetos no Painel de Seleção

**Organizar > Painel de Seleção** abre uma lista de todos os objetos da página, com o que está à frente em primeiro lugar. É útil quando os objetos se ocultam uns aos outros.

- Clique num nome para selecionar esse objeto.
- Faça duplo clique num nome, ou prima **F2**, para escrever um novo nome para o objeto. Um objeto sem nome próprio mostra o seu tipo e número, como Forma 2.
- Desmarque a caixa ao lado de um nome para ocultar o objeto. Um objeto oculto não é desenhado na página, numa impressão ou num PDF, e um clique na página não o seleciona. Marque a caixa para o voltar a mostrar. **Mostrar Tudo** e **Ocultar Tudo** definem todas as caixas de uma só vez.
- Selecione um nome e clique em **Trazer para a Frente** ou **Enviar para Trás** para mover o objeto uma posição na lista, ou arraste um nome para cima ou para baixo.

Cada uma destas ações pode ser anulada. Um nome e a marca de ocultação ficam guardados num ficheiro .jpub. Um ficheiro .pub não tem espaço para eles.

## Bloquear um objeto

Clique com o botão direito num objeto e escolha **Bloquear Posição e Tamanho**. Não pode ser movido nem redimensionado, e as suas alças passam a marcas X vermelhas. Escolha novamente para desbloquear.

## Consulte também

- [Contornar objetos com texto](wrap)
- [Réguas, guias e ajuste](guides)
- [O separador Formato da Forma](tab-shape-format)
