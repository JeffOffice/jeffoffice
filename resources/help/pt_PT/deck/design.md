# Tamanho do diapositivo, fundo e rodapés
<!-- keywords: estrutura, tema, temas, variantes, variante, cores, tipos de letra, criar novas cores do tema, criar novos tipos de letra do tema, cores personalizadas, ficheiro de tema, thmx, guardar tema atual, repor tema, estilos de fundo, modelo global de diapositivos, esquemas, procurar temas, aplicar a todos os diapositivos, aplicar aos diapositivos selecionados, tamanho do diapositivo, ecrã panorâmico, 16:9, padrão, 4:3, proporção, ajustar ao conteúdo, tamanho personalizado de diapositivo, numerar diapositivos a partir de, primeiro número do diapositivo, iniciar em, fundo, cor de fundo, formatar fundo, cabeçalho, rodapé, número do diapositivo, data, diapositivo de título, tipo de letra, tipografia, marcas de página, ocultar gráficos de fundo>

O separador **Estrutura** altera o aspeto de todos os diapositivos: o seu tema, a sua forma, o seu fundo e o rodapé ao longo da margem inferior.

## Tamanho do diapositivo

Clique em **Estrutura > Personalizar > Tamanho do Diapositivo** e escolha um tamanho:

- **Padrão (4:3)** tem 25,4 cm de largura e 19,05 cm de altura. É a forma mais quadrada dos ecrãs e projetores mais antigos.
- **Ecrã Panorâmico (16:9)** tem 33,867 cm de largura e 19,05 cm de altura. As novas apresentações começam com este tamanho.

Uma marca de verificação indica o tamanho que a apresentação tem neste momento. Quando escolhe o outro, o JeffDeck pergunta **Pretende dimensionar o conteúdo para caber no novo tamanho de diapositivo?** Clique em **Ajustar** para alterar o tamanho. O texto e a espessura das linhas são dimensionados, os esquemas e os marcadores de posição são esticados para a nova forma, e os restantes objetos são dimensionados para manter a sua própria forma e ficar centrados. Clique em **Cancelar** para não alterar nada. Ctrl+Z anula a alteração.

Uma apresentação aberta a partir de um ficheiro pode ter um tamanho que não é nenhum destes. Mantém esse tamanho, e nenhuma das opções fica marcada.

**Tamanho Personalizado do Diapositivo** tem uma lista de tamanhos, uma largura e uma altura, a opção Horizontal ou Vertical, e a forma como as páginas de notas são orientadas. **Numerar diapositivos a partir de** é o número a partir do qual conta o primeiro diapositivo, de modo que o número do diapositivo, nas páginas impressas e nas páginas de um ficheiro .pptx guardado, comece aí. Pode ser qualquer número inteiro, zero ou negativo também, até cerca de dois mil milhões em qualquer sentido; um ficheiro de outro programa com um número acima desse limite é aberto com o mais próximo, e as notas à abertura indicam-no. Os números dos diapositivos e das páginas impressas mostram esse valor. Um marcador PDF de um diapositivo com título é o próprio título, com o número do diapositivo à frente quando a numeração não começa em 1 ("7. Título"), e um marcador de um diapositivo sem título é "Slide 7"; as imagens que exportar continuam a ser nomeadas pela posição do diapositivo ("Slide 1.png", "Slide 2.png"), para ficarem ordenadas. Um ficheiro .odp não guarda o número, e Verificar Compatibilidade indica-o. Alterar o tamanho, o número ou as páginas de notas é um passo de Ctrl+Z.

## Fundo

Clique em **Estrutura > Personalizar > Formatar Fundo**. Escolha uma **Cor de preenchimento sólida** entre as cores do tema ou uma cor sua. Depois clique em:

- **Aplicar** para alterar apenas o diapositivo em que está.
- **Aplicar a Todos** para alterar todos os diapositivos. Os diapositivos e esquemas que tinham um fundo próprio passam a seguir o novo.

Marque **Ocultar gráficos de fundo** para impedir que as imagens e formas do modelo global de diapositivos apareçam no diapositivo em que está. A cor de fundo mantém-se. Na vista Modelo Global de Diapositivos, a caixa aplica-se ao esquema que está a editar.

## Cabeçalho e rodapé

Um rodapé é uma linha de texto pequeno ao longo da parte inferior de um diapositivo. Pode conter a data, o número do diapositivo e uma linha de texto da sua autoria. Aparece no diapositivo, por isso também é impresso e reproduzido.

1. Clique em **Inserir > Texto > Cabeçalho e Rodapé**. A janela tem dois separadores, **Diapositivo** e **Notas e Folhetos**.
2. No separador **Diapositivo**, em **Incluir no diapositivo**, marque o que pretende: **Data e hora**, **Número do diapositivo** e **Rodapé**. Para **Rodapé**, escreva o texto na caixa ao lado.
3. Marque **Não mostrar no diapositivo de título** para deixar o primeiro diapositivo limpo.
4. Clique em **Aplicar** para o diapositivo em que está, ou em **Aplicar a Todos** para todos os diapositivos. Com **Aplicar a Todos**, os diapositivos que adicionar mais tarde também recebem o rodapé.

O separador **Notas e Folhetos** tem a **Data**, o **Cabeçalho**, o **Rodapé** e o **Número de página** que são impressos no topo e na base das páginas de notas e dos folhetos. Marque o que pretende e escreva o texto ao lado; deixe **Data** vazia para imprimir a data de hoje. Estes elementos vão em conjunto para as páginas de notas e para os folhetos, e **Aplicar** e **Aplicar a Todos** guardam-nos juntamente com as opções do separador **Diapositivo**, como um único passo de Anular. As mesmas opções estão em **Cabeçalho e Rodapé** na página Imprimir do ecrã Ficheiro.

A data e o número do diapositivo são campos, que se mantêm corretos por si: a data é a de hoje, e o número acompanha a posição do diapositivo quando o move.

Para colocar o título da apresentação, o autor, o nome do ficheiro ou o número de diapositivos no texto de uma caixa, clique na caixa e depois escolha **Inserir**, **Campo** e o que pretende. Cada um é um campo, por isso muda quando a apresentação muda: edite o título em **Ficheiro**, **Informações** e o texto acompanha-o. **Número de Diapositivos** conta todos os diapositivos da apresentação, incluindo os ocultos, e mostra o mesmo número em todos os diapositivos e em todas as secções. Um ficheiro guardado mantém os quatro como campos: mostram as palavras que tinham ao ser guardados, e voltam a acompanhar a apresentação quando o abre no JeffDeck. Outro programa pode mostrar apenas as palavras. Quando **Remover informações pessoais deste ficheiro ao guardar** está ativo, um campo **Autor** é guardado sem texto, por isso o ficheiro não contém o nome de nenhum autor; o campo continua presente quando abre o ficheiro. **Inspecionar Documento** indica quantos diapositivos têm um campo Autor. O número do diapositivo e a data continuam a ser campos nos dois tipos de ficheiro.

## Temas

Um **tema** é o aspeto completo de uma apresentação: as suas cores, os seus tipos de letra, os seus fundos, e o **modelo global de diapositivos** e os **esquemas** com o que desenham. O modelo global de diapositivos é a página por trás de todos os diapositivos. Um esquema é um plano para um tipo de diapositivo, como Diapositivo de Título ou Dois Conteúdos. Os temas desenham faixas, linhas e formas sobre eles.

O JeffDeck tem nove temas próprios:

- **JeffOffice** é o tema branco e simples com que começa uma nova apresentação.
- **Harbor** tem uma faixa azul-marinho no topo e uma linha azul-petróleo por baixo.
- **Ember** é escuro, com uma barra âmbar numa das margens e títulos em maiúsculas.
- **Almanac** parece papel quente, com um pequeno separador e uma linha dupla sob cada título.
- **Confetti** é vivo, com triângulos coloridos nos cantos.
- **Granite** é escuro e frio, com uma faixa de ardósia por trás do título.
- **Orchard** é verde suave, com um painel de título arredondado e círculos grandes.
- **Marquee** é preto e dourado, com uma moldura dupla fina.
- **Tideline** é azul pálido, com faixas de água no topo.

### Escolher um tema

1. Clique no separador **Estrutura**. O grupo **Temas** mostra uma imagem do diapositivo de título de cada tema.
2. Clique numa imagem. O tema aplica-se a todos os diapositivos, num único passo. Ctrl+Z anula-o.

As setas para cima e para baixo ao lado das imagens mostram a linha seguinte de temas. O último botão abre os nove de uma vez. O tema da sua apresentação aparece com contorno.

Os seus diapositivos mantêm o que têm. Cada diapositivo passa para o esquema do seu próprio tipo, por isso um Diapositivo de Título continua a ser um Diapositivo de Título e um diapositivo Dois Conteúdos continua a ser Dois Conteúdos. As palavras mantêm-se, e as caixas passam para os lugares do novo esquema. O texto que usava os tipos de letra e as cores do tema antigo passa a usar os do novo tema. O texto que definiu manualmente mantém o seu aspeto: uma palavra vermelha, uma linha a negrito ou um tipo de letra que escolheu. Uma alteração de tema ou de fundo analisa também o que está por baixo de cada texto. O texto numa célula de tabela ou numa forma com uma cor própria, ou sobre uma imagem, não muda para claro ou escuro com o fundo do diapositivo. E se uma cor que definiu ficar difícil de ler no novo fundo (abaixo de 4,5 para 1), aproxima-se do claro ou do escuro, apenas o suficiente para ser legível.

Uma imagem num marcador de posição passa para o lugar correspondente do novo esquema e nunca é esticada. Num marcador de posição de imagem é recortada para preencher o lugar, a partir do centro. Num marcador de posição de conteúdo, a imagem inteira é ajustada dentro do lugar, para que nada fique cortado. Uma imagem que não está num marcador de posição mantém-se onde a colocou.

### Clicar com o botão direito num tema

Clique com o botão direito numa imagem para ver três opções:

- **Aplicar a Todos os Diapositivos** é o mesmo que um clique.
- **Aplicar aos Diapositivos Selecionados** altera apenas os diapositivos que selecionou (na Organização de Diapositivos) ou o diapositivo em que está. Esses diapositivos recebem um modelo global de diapositivos próprio, e os restantes mantêm o seu. Se escolher mais tarde o mesmo tema e variante para outros diapositivos, passam para esse mesmo modelo global, em vez de receberem outra cópia dele.
- **Procurar Temas…** permite escolher um ficheiro de apresentação ou de modelo (.pptx, .potx, .ppsx, .ppt, .pps ou .pot) e usa o seu tema em todos os diapositivos. Se o ficheiro não tiver modelo global de diapositivos, ou se o modelo não tiver esquemas, o JeffDeck indica-o e não altera nada. Também pode escolher um ficheiro de tema (.thmx) ou um documento (.docx ou .dotx). Um ficheiro destes não tem diapositivos nem esquemas, apenas as cores e os tipos de letra de um tema; por isso os seus diapositivos mantêm os esquemas e passam a usar as novas cores e os novos tipos de letra, num único passo.
- **Guardar Tema Atual…** grava as doze cores do tema, os seus dois tipos de letra e os seus estilos de efeito num ficheiro de tema (.thmx). Outras aplicações JeffOffice e outros programas podem usar o tema a partir dele.

Um tema criado para diapositivos de outra forma (panorâmica, ou 4 por 3) é esticado para se ajustar ao seu. Os círculos e os quadrados do desenho mantêm-se redondos e quadrados, e as faixas e as linhas continuam a chegar às margens.

## Variantes

**Variantes** são outros aspetos do mesmo tema. O grupo **Variantes** mostra quatro pequenas imagens. Cada uma tem uma linha de barras coloridas na base: as suas cores. Clique numa para a usar. Altera as cores e, quando se aplica, também o fundo.

A seta à direita do grupo abre um menu:

- **Cores** mostra as doze cores que o seu tema tem agora. Por baixo está uma lista de esquemas de cores: primeiro os que criou, depois os incorporados. Clique num, e cada cor do esquema antigo passa a ser a cor correspondente do novo, onde quer que tenha sido usada. As cores que escolheu manualmente mantêm-se.
- **Tipos de Letra** lista pares de tipos de letra: um para os títulos e um para o texto corrente. Clique num par, e o texto que usava os tipos de letra antigos passa a usar os novos.
- **Estilos de Fundo** mostra doze amostras. Na horizontal estão as quatro cores do tema: Clara 1, Clara 2, Escura 2 e Escura 1. Na vertical estão três tipos: liso, uma gradação suave e uma gradação mais forte. Uma gradação passa gradualmente de uma cor para outra. Clique numa amostra para a usar em todos os diapositivos. Os fundos que os diapositivos e os esquemas tinham próprios cedem. Num fundo escuro, o texto fica claro, e num fundo claro fica escuro. As hiperligações também mudam, para continuarem fáceis de ler: o JeffDeck escolhe cores de ligação que se leem sobre o fundo e, sempre que possível, sobre as faixas e os painéis que alguns esquemas desenham por trás do texto.

Cada escolha é um passo que Ctrl+Z pode anular.

### Criar as suas próprias cores e tipos de letra

Na parte inferior da lista **Cores**, clique em **Criar Novas Cores do Tema…**. A caixa tem um botão para cada uma das doze cores. Clique num botão para escolher uma cor para esse lugar. A faixa por baixo dos botões mostra as cores com um título de exemplo, uma linha de texto e uma hiperligação. Escreva um nome e clique em **Guardar**. As cores passam para os seus diapositivos de imediato, e ficam na lista. **Criar Novos Tipos de Letra do Tema…**, na parte inferior da lista **Tipos de Letra**, funciona da mesma forma: escolha um tipo de letra para os títulos e outro para o texto, verifique o exemplo, dê um nome ao par e guarde. Não pode usar um nome que um conjunto incorporado já tenha.

Os seus conjuntos ficam guardados na pasta de definições, por isso o JeffWrite também os mostra. Para retirar um da lista, use **Eliminar Cores Personalizadas** ou **Eliminar Tipos de Letra Personalizados** no mesmo menu.

### Repor o tema do modelo

**Repor o Tema do Modelo**, na parte inferior das listas **Cores** e **Tipos de Letra**, repõe as cores e os tipos de letra que a apresentação tinha quando a abriu, ou quando a criou. É um passo, e Ctrl+Z anula-o. A opção fica a cinzento quando nada foi alterado.

## Guardar

Uma apresentação mantém o seu tema, modelo global de diapositivos e esquemas ao ser guardada. Outros programas de apresentações leem-nos, e o JeffDeck volta a lê-los exatamente como estavam. Um tema ou estilo de fundo escuro é guardado da forma como outros programas guardam os seus temas escuros: a página conta como escura, por isso uma caixa de texto que acrescente a um diapositivo aí começa clara. Para alterar o modelo global de diapositivos ou um esquema, use a vista Modelo Global de Diapositivos, no separador **Ver**.

## Relacionados

- [Separador Estrutura](tab-design)
- [Adicionar, alterar e eliminar diapositivos](slides)
- [Reutilizar diapositivos de outra apresentação](reuse)
