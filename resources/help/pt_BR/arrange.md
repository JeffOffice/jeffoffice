# Mover, alinhar, agrupar e sobrepor objetos
<!-- keywords: painel de seleção, ocultar objeto, mostrar objeto, renomear objeto, nome do objeto, selecionar, mover, redimensionar, girar, inverter, alinhar, distribuir, agrupar, desagrupar, camada, trazer para a frente, enviar para trás, deslocar, bloquear, duplicar -->

Cada imagem, caixa de texto, forma e linha em uma página é um objeto. Este tópico mostra como selecionar, mover, redimensionar e sobrepor objetos.

## Selecionar objetos

- Clique em um objeto. Para selecionar uma caixa de texto, clique dentro dela e depois pressione **Esc**.
- Mantenha **Shift** pressionada e clique para adicionar um objeto à seleção ou para removê-lo.
- Arraste um retângulo em uma parte vazia da página para envolver os objetos. Os objetos que cabem completamente dentro do retângulo são selecionados.
- Pressione **Ctrl+A** para selecionar todos os objetos da página. Pressione **Tab** para passar para o próximo objeto.
- **Página Inicial > Edição > Selecionar** pode selecionar todas as caixas de texto, imagens, formas, tabelas ou Texto Artístico de uma só vez.

## Mover e redimensionar

- Arraste um objeto para movê-lo. Mantenha **Shift** pressionada para movê-lo somente em linha reta, na horizontal ou na vertical. Mantenha **Alt** pressionada para desativar o ajuste. Mantenha **Ctrl** pressionada enquanto arrasta para criar uma cópia.
- Pressione as teclas de seta para deslocar o objeto em um oitavo de polegada (cerca de 0,32 cm). **Alt** com uma tecla de seta move o objeto um pixel da tela. Você pode alterar a distância do deslocamento em **Arquivo > Opções > Avançado**.
- Arraste uma alça quadrada para redimensionar. Uma alça de canto altera largura e altura ao mesmo tempo. Uma alça lateral altera apenas uma delas.
- Caixas de texto e formas: mantenha **Shift** pressionada enquanto arrasta um canto para manter as proporções.
- Imagens, Texto Artístico, ícones e grupos já mantêm as proporções ao arrastar um canto. Mantenha **Shift** pressionada para esticá-los livremente.
- Mantenha **Ctrl** pressionada para redimensionar a partir do centro.
- Para valores exatos, digite nas caixas **Altura** e **Largura** do grupo **Tamanho** (em guias como **Formato da Forma**), ou clique com o botão direito no objeto, escolha **Formatar Objeto…** e use as guias **Tamanho** e **Layout**. **Exibir > Ferramentas > Medida** abre uma pequena janela que mostra e define posição, tamanho e rotação.

## Girar e inverter

Arraste a alça verde redonda acima de um objeto selecionado. Mantenha **Shift** pressionada para girar em passos de 15 graus. Ou clique em **Página Inicial > Organizar > Girar** e escolha **Girar 90° à Direita**, **Girar 90° à Esquerda**, **Inverter Verticalmente**, **Inverter Horizontalmente** ou **Mais Opções de Rotação…** para digitar um ângulo.

## Alinhar e distribuir

1. Selecione os objetos.
2. Clique em **Página Inicial > Organizar > Alinhar**.
3. Escolha Alinhar à Esquerda, Centralizar, Alinhar à Direita, Alinhar em Cima, Alinhar ao Meio ou Alinhar Embaixo.

Com vários objetos selecionados, eles se alinham entre si. Com um único objeto selecionado, ele se alinha às bordas da página. Marque **Em Relação às Guias de Margem** no mesmo menu para usar as guias de margem.

**Distribuir Horizontalmente** e **Distribuir Verticalmente** espaçam três ou mais objetos de forma uniforme entre os dois mais externos.

## Dar o mesmo tamanho aos objetos

1. Selecione dois ou mais objetos. Selecione primeiro aquele cujo tamanho os outros devem assumir.
2. Clique em **Página Inicial > Organizar > Alinhar**.
3. Escolha **Igualar Largura**, **Igualar Altura** ou **Equalizar Largura e Altura**.

Cada um dos outros objetos assume a largura e a altura daquele (um objeto girado assume sua própria largura e altura, e não as do contorno girado) e mantém o canto superior esquerdo onde estava. Um objeto bloqueado mantém o próprio tamanho. Cada escolha é uma etapa de Desfazer.

## Raio do canto e o ponto de redimensionamento

**Formatar Objeto… > Tamanho** tem **Redimensionar a partir de**. Ele indica o ponto do objeto que permanece no lugar quando você altera a altura ou a largura: um canto, o meio de um lado ou o centro. A escolha usual é **Superior esquerdo**. Em um objeto girado, o ponto permanece onde você o vê na página. Se você também digitar uma nova posição, a posição digitada prevalece. Para um retângulo de cantos arredondados (e as formas com um ou dois cantos arredondados), **Raio do canto** define o quanto os cantos são arredondados, na unidade que você usa. O valor máximo é metade do lado menor, onde os cantos se encontram.

## Duplicar várias vezes

Ctrl+D cria uma cópia, deslocada 12 pontos para baixo e para a direita. Para criar mais cópias, selecione os objetos e clique em **Página Inicial > Organizar > Duplicar Várias Vezes…** (ou clique com o botão direito no objeto e escolha essa opção). Na janela, defina:

- **Número de cópias**, de 1 a 200. No máximo 2.000 novos objetos são criados por vez, e as cópias param antes da primeira que ficaria maior que 20 vezes o lado maior da página, menor que um ponto ou deslocada além disso.
- **Deslocamento horizontal** e **Deslocamento vertical**: a distância que cada cópia é deslocada em relação à anterior. Um número negativo move a cópia para a esquerda ou para cima.
- **Alteração de tamanho para cada cópia**, em porcentagem. Por exemplo, 10 % faz cada cópia ficar 10 % maior que a anterior, e -10 % faz cada uma ficar menor. O canto superior esquerdo de cada cópia fica onde o seu deslocamento o coloca.
- **Variar gradualmente a cor de preenchimento para**: escolha uma cor, e as cópias mudam em etapas, do preenchimento sólido do próprio objeto até essa cor, de modo que a última cópia tenha exatamente essa cor. Um objeto sem preenchimento sólido mantém o seu preenchimento.

Todas as cópias são uma etapa de Desfazer, e as últimas cópias ficam selecionadas.

## Enviar objetos para a página mestra

1. Selecione os objetos de uma página.
2. Clique em **Página Inicial > Organizar > Enviar para Página Mestra**.

Os objetos passam para a página mestra usada pela página, na frente do que já existe nela, e continuam com a mesma aparência: permanecem no mesmo lugar da página. Depois disso, eles aparecem em todas as páginas que usam a mestra, e você os altera na página mestra. Para trazê-los de volta, abra a página mestra (**Exibir > Modos de Exibição > Página Mestra**), selecione-os e clique em **Enviar para Página** (o mesmo botão). Eles voltam para a página em que você estava. Os objetos saem da página, por isso aparecem em todas as páginas que usam essa mestra, e não só nesta. Cada ação é uma etapa de Desfazer. Uma página sem página mestra, um objeto bloqueado, um objeto dentro de um grupo (selecione o grupo inteiro), uma caixa de texto vinculada a outra e um objeto preso à sua página (um gráfico ou um objeto inserido) permanecem onde estão, e a barra de status informa o motivo.

## Agrupar objetos

Um grupo se move, redimensiona e gira como um único objeto.

1. Selecione dois ou mais objetos.
2. Clique em **Página Inicial > Organizar > Agrupar** (Ctrl+Shift+G).

Para desfazer um grupo, selecione-o e clique em **Desagrupar**. A seta ao lado de **Desagrupar** oferece **Reagrupar**, que junta novamente o último grupo. Para selecionar um objeto dentro de um grupo, clique duas vezes nele. Pressione **Esc** para selecionar o grupo inteiro novamente.

## Alterar a ordem de empilhamento

Os objetos se empilham como folhas de papel. Use **Avançar** e **Recuar** para mover um objeto uma posição. A seta ao lado de cada botão oferece **Trazer para a Frente** (Alt+F6) e **Enviar para Trás** (Alt+Shift+F6). Clique com o botão direito e escolha **Ordem** para as mesmas opções.

## Listar os objetos no Painel de Seleção

**Organizar > Painel de Seleção** abre uma lista de todos os objetos da página, com o que está na frente em primeiro lugar. Ele ajuda quando objetos se escondem uns atrás dos outros.

- Clique em um nome para selecionar esse objeto.
- Clique duas vezes em um nome, ou pressione **F2**, para digitar um novo nome para o objeto. Um objeto sem nome próprio mostra seu tipo e número, como Forma 2.
- Desmarque a caixa ao lado de um nome para ocultar o objeto. Um objeto oculto não é desenhado na página, em uma impressão ou em um PDF, e um clique na página não o seleciona. Marque a caixa para mostrá-lo novamente. **Mostrar Tudo** e **Ocultar Tudo** definem todas as caixas de uma vez.
- Selecione um nome e clique em **Avançar** ou **Recuar** para mover o objeto uma posição na lista, ou arraste um nome para cima ou para baixo.

Cada uma dessas ações pode ser desfeita. Um nome e a marca de ocultação são salvos em um arquivo .jpub. Um arquivo .pub não tem espaço para eles.

## Bloquear um objeto

Clique com o botão direito em um objeto e escolha **Bloquear Posição e Tamanho**. Ele não pode ser movido nem redimensionado, e suas alças se transformam em marcas X vermelhas. Escolha a opção novamente para desbloquear.

## Veja também

- [Quebrar texto ao redor de objetos](wrap)
- [Réguas, guias e ajuste](guides)
- [A guia Formato da Forma](tab-shape-format)
