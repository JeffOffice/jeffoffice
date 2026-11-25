# Separador Base

<!-- keywords: separador base, friso, área de transferência, grupo diapositivos, novo diapositivo, esquema, repor, secção, eliminar diapositivo, grupo tipo de letra, grupo parágrafo, grupo desenho, formas, organizar, preenchimento da forma, contorno da forma, ferramentas de formatação, grupo edição, localizar, substituir, selecionar, selecionar tudo, selecionar objetos -->

O separador **Base** reúne as ferramentas que mais utiliza ao criar diapositivos.

- **Área de Transferência** tem **Colar** (a seta tem **Manter Apenas Texto**, que cola palavras sem formatação, e **Colar Especial** (Ctrl+Alt+V), que apresenta as formas como a área de transferência pode ser colada: formatação de origem, apenas texto ou uma imagem), **Cortar**, **Copiar** e **Copiar Formatação**, que copia o aspeto de um texto ou de um objeto para outro. Logo após colar num diapositivo, surge um pequeno botão **Opções de Colagem** junto ao que colou. Clique nele para escolher **Manter Formatação de Origem**, **Manter Apenas Texto**, **Imagem** (quando a área de transferência tem uma imagem) ou **Células da Folha de Cálculo como Tabela** (quando tem células). O JeffDeck desfaz a colagem e refaz-na desse modo, e um único **Anular** desfaz a colagem inteira, por mais vezes que mude de ideias. A partir do teclado, prima a tecla **Ctrl** (a tecla **Control** num Mac) depois de colar para abrir o menu do botão, e depois use as teclas de seta e **Enter** para escolher. O botão desaparece quando altera alguma coisa, seleciona outra coisa, copia outra coisa, prime **Esc** ou faz deslocar o objeto colado para fora da vista. **Definir Colagem Predefinida** está na página **Avançadas** de **Opções**: uma caixa para colar dentro da mesma apresentação, entre apresentações e a partir de outros programas, com **Manter Formatação de Origem** ou **Manter Apenas Texto**. Consulte [Formatar texto](text).
- **Diapositivos** tem **Novo Diapositivo** (Ctrl+M, com uma seta para os esquemas e **Reutilizar Diapositivos…**), **Esquema**, **Repor**, **Secção** e **Eliminar Diapositivo**. Consulte [Adicionar, alterar e eliminar diapositivos](slides).
- **Tipo de Letra** altera o tipo de letra, o tamanho, a cor e o estilo das letras, o respetivo espaçamento e as maiúsculas e minúsculas. Consulte [Formatar texto](text).
- **Parágrafo** tem marcas, numeração, níveis de lista, espaçamento entre linhas, colunas, alinhamento, direção do texto e a posição do texto na respetiva caixa. Consulte [Formatar texto](text).
- **Desenho** tem **Formas**, **Organizar**, **Preenchimento da Forma** e **Contorno da Forma**. Consulte [Formas, imagens, tabelas e Texto Artístico](objects). O separador **Formato da Forma** e o painel **Formatar Forma** (Ctrl+1) oferecem mais opções.
- **Edição** tem **Localizar** (Ctrl+F), **Substituir** (Ctrl+H), **Ir Para** (Ctrl+G) e **Selecionar**. Localizar e Substituir abrem um painel à direita que pesquisa em todos os diapositivos e nas respetivas notas. **Ir Para** abre uma janela com uma lista **Ir para o quê**: **Diapositivo**, **Secção**, **Comentário** ou **Objeto**. Também pode ir para uma secção, um comentário ou um objeto. **Selecionar** tem **Selecionar Tudo** (Ctrl+A) e **Selecionar Objetos**, que volta ao ponteiro depois de desenhar.

## Relacionados

- [Formatar texto](text)
- [Adicionar, alterar e eliminar diapositivos](slides)
- [Formas, imagens, tabelas e Texto Artístico](objects)

## O painel Área de Transferência

O painel **Área de Transferência** guarda as últimas 24 coisas que copiou ou recortou nesta janela do programa, para que possa colar uma mais antiga sem a copiar de novo. Clique na pequena seta no canto do grupo **Área de Transferência** no separador **Base** (ou prima **Alt**, depois **H** e, em seguida, **FO**) para o abrir. Um painel aparece ao lado da página.

Cada item mostra uma pequena pré-visualização e o tipo de conteúdo: texto, texto formatado, células, uma imagem, formas ou diapositivos. O mais recente fica no topo. Clique num item para o colar onde estiver o cursor ou a seleção. Cola-se como qualquer outra colagem: um passo de **Anular** desfaz-o, e o botão **Opções de Colagem** aparece como de costume.

- **Colar Tudo** cola todos os itens, do mais antigo para o mais recente, para que fiquem na ordem pela qual os copiou.
- **Eliminar** remove o item escolhido. **Limpar Tudo** remove todos.
- O painel só recolhe itens enquanto estiver aberto. Para continuar a recolher depois de o fechar, marque **Continuar a recolher quando este painel for fechado**. A escolha fica guardada, e a recolha começa quando o programa é iniciado, antes de abrir o painel.
- Para recolher também o que copia noutros programas, marque **Recolher também o que é copiado noutros programas**. Só são guardados texto simples e imagens, e esta opção é desativada novamente sempre que abre o programa. Com esta opção ativa, uma cópia feita noutro programa é lida por completo antes de este programa decidir se a guarda; por isso, uma cópia muito grande pode fazer este programa parar por um momento e usar tanta memória quanto o tamanho da cópia. Só é lida uma forma dela: o respetivo texto, quando o tem, caso contrário uma imagem. Sem esta opção, nada copiado noutros programas é lido.

Os itens são guardados apenas na memória do programa. Nada é gravado num ficheiro, e a lista é esvaziada quando fecha a última janela do programa. Um item permanece na lista depois de o documento de origem ser fechado. Um item com mais de 8 MB não é guardado, e o painel indica quando isso acontece; uma imagem conta 4 bytes por cada ponto (píxel), pelo que uma imagem com mais de cerca de 2 milhões de píxeis é demasiado grande. Todos os itens juntos ficam limitados a 64 MB, e os mais antigos são removidos primeiro. Qualquer conteúdo que um gestor de palavras-passe marque como secreto nunca é guardado.
